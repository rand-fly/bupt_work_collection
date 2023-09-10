#include "GlobalContext.h"
#include "ui.h"

using namespace ftxui;

class MakePageBase : public PageBase {
  public:
    MakePageBase(GlobalContext &ctx) : PageBase(ctx) {
        InputOption inputOption;
        inputOption.on_enter = std::bind(&MakePageBase::submit, this);
        m_input = Input(&m_inputText, "输入单词", inputOption);

        ButtonOption option = ButtonOption::Ascii();

        auto buttonSubmit = Button(
            "提交", std::bind(&MakePageBase::submit, this), option);

        auto buttonBack = Button(
            "退出", [&] { switchPage(MainPage(m_ctx)); }, option);
        m_buttons = Container::Horizontal({buttonSubmit, buttonBack});

        Add(Container::Vertical({m_input, m_buttons}));
    }

  private:
    Element Render() override {
        std::string info = " " + std::to_string(m_ctx.user.level) + "级 出题数"
                         + std::to_string(m_ctx.user.madeNum)
                         + "/" + std::to_string(m_ctx.user.madeNumForNextLevel) + " ";
        return vbox({text(info),
                     filler(),
                     m_input->Render() | size(WIDTH, GREATER_THAN, 20) | hcenter,
                     filler(),
                     m_buttons->Render() | hcenter,
                     filler()})
             | size(WIDTH, EQUAL, 90)
             | size(HEIGHT, EQUAL, 30)
             | border;
    }

    void submit() {
        if (m_inputText.empty()) {
            alert("单词不能为空");
            return;
        }

        m_socket.write("make_problem\n"
                      + m_inputText + "\n");
        auto is = m_socket.readStream();
        std::string line;
        std::getline(is, line);
        std::getline(is, line);
        if (line == "success") {
            alert("提交成功");
            m_inputText.clear();
            m_input->TakeFocus();
        } else {
            alert(line);
        }
        is >> m_ctx.user.level
            >> m_ctx.user.madeNum
            >> m_ctx.user.madeNumForNextLevel;
    }

    std::string m_inputText;
    Component m_input, m_buttons;
};

Component MakePage(GlobalContext &ctx) {
    return Make<MakePageBase>(ctx);
}
