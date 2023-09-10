#include "GlobalContext.h"
#include "ui.h"
using namespace ftxui;

class SignupPageBase : public PageBase {
  public:
    SignupPageBase(GlobalContext &ctx) : PageBase(ctx) {
        const static std::vector<std::string> toggleList = {"闯关者", "出题者"};
        m_toggleType = Toggle(&toggleList, &m_type);

        InputOption password_option;
        password_option.password = true;
        m_inputName = Input(&m_name, "用户名");
        m_inputPassword = Input(&m_password, "密码", password_option);
        m_inputConfirmPassword = Input(&m_confirmPassword, "确认密码", password_option);

        ButtonOption option = ButtonOption::Ascii();
        auto buttonOK = Button("确认", std::bind(&SignupPageBase::onEnter, this), option);
        auto buttonBack = Button(
            "返回", [&] { m_ctx.router->switchPage(LoginPage(m_ctx)); }, option);
        m_buttons = Container::Horizontal({buttonOK, buttonBack});

        auto child = Container::Vertical({m_toggleType,
                                          m_inputName,
                                          m_inputPassword,
                                          m_inputConfirmPassword,
                                          m_buttons});

        Add(child);
    }

  private:
    Element Render() override {
        return vbox({text(" 请注册") | hcenter,
                     separator(),
                     hbox(text(" 用户类别 : "), m_toggleType->Render()),
                     hbox(text(" 用户名   : "), m_inputName->Render()),
                     hbox(text(" 密码     : "), m_inputPassword->Render()),
                     hbox(text(" 确认密码 : "), m_inputConfirmPassword->Render()),
                     separator(),
                     m_buttons->Render() | hcenter})
             | border | size(WIDTH, GREATER_THAN, 40);
    }

    void onEnter() {
        if (m_password != m_confirmPassword) {
            alert("两次输入的密码不一致");
            return;
        }
        if (m_name.empty()) {
            alert("用户名不能为空");
            return;
        }
        if (m_password.empty()) {
            alert("密码不能为空");
            return;
        }
        m_socket.write("signup\n"
                      + std::to_string(m_type + 1) + "\n"
                      + m_name + "\n"
                      + m_password + "\n");

        
        auto is = m_socket.readStream();
        std::string line;
        std::getline(is, line);
        std::getline(is, line);
        if (line == "success") {
            alert("注册成功");
            m_ctx.router->switchPage(LoginPage(m_ctx));
        } else {
            alert(line);
        }
    }

    int m_type = 0;
    std::string m_name, m_password, m_confirmPassword;
    std::string m_errorMessage;
    Component m_toggleType, m_inputName, m_inputPassword, m_inputConfirmPassword, m_buttons;
};

Component SignupPage(GlobalContext &context) {
    return Make<SignupPageBase>(context);
}
