#include "GlobalContext.h"
#include "ui.h"

using namespace ftxui;

class LoginPageBase : public PageBase {
  public:
    LoginPageBase(GlobalContext &ctx) : PageBase(ctx) {
        InputOption password_option;
        password_option.password = true;
        m_inputName = Input(&m_name, "用户名");
        m_inputPassword = Input(&m_password, "密码", password_option);

        ButtonOption option = ButtonOption::Ascii();
        auto buttonOK = Button("确认", std::bind(&LoginPageBase::onEnter, this), option);
        auto buttonSignin = Button(
            "注册", [&] { m_ctx.router->switchPage(SignupPage(m_ctx)); }, option);
        auto buttonExit = Button(
            "退出", [&] { m_ctx.screen.Exit(); }, option);
        m_buttons = Container::Horizontal({buttonOK, buttonSignin, buttonExit});
        auto child = Container::Vertical({m_inputName, m_inputPassword, m_buttons});
        Add(child);
    }

  private:
    Element Render() override {
        return vbox({
                   text(" 请登录") | hcenter,
                   separator(),
                   hbox(text(" 用户名 : "), m_inputName->Render()),
                   hbox(text(" 密码   : "), m_inputPassword->Render()),
                   separator(),
                   m_buttons->Render() | hcenter,
               })
             | border | size(WIDTH, GREATER_THAN, 40);
    }

    void onEnter() {
        if (m_name.empty()) {
            alert("用户名不能为空");
            return;
        }
        if (m_password.empty()) {
            alert("密码不能为空");
            return;
        }
        m_socket.write("login\n"
                      + m_name + "\n"
                      + m_password + "\n");
        std::string _;
        auto is = m_socket.readStream();
        std::string line;
        std::getline(is, _);
        std::getline(is, line);
        if (line == "success") {
            int type;
            is >> type;
            std::getline(is, _);
            m_ctx.user.name = m_name;
            m_ctx.user.type = type;
            if (type == 1) {
                is >> m_ctx.user.level
                    >> m_ctx.user.exp
                    >> m_ctx.user.expForNextLevel
                    >> m_ctx.user.levelPassed;
            } else if (type == 2) {
                is >> m_ctx.user.level
                    >> m_ctx.user.madeNum
                    >> m_ctx.user.madeNumForNextLevel;
            }
            m_ctx.router->switchPage(MainPage(m_ctx));
        } else {
            alert(line);
        }
    }

    std::string m_name, m_password;
    Component m_inputName, m_inputPassword, m_buttons;
};

Component LoginPage(GlobalContext &ctx) {
    return Make<LoginPageBase>(ctx);
}
