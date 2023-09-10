#include "GlobalContext.h"
#include "ui.h"
#include <thread>

using namespace ftxui;

class PlayPageBase : public PageBase {
  public:
    PlayPageBase(GlobalContext &ctx) : PageBase(ctx) {
        InputOption inputOption;
        inputOption.on_enter = std::bind(&PlayPageBase::submit, this);
        m_input = Input(&m_inputText, "输入单词", inputOption);

        m_input |= Maybe([&] { return m_state == State::input; });

        ButtonOption option = ButtonOption::Ascii();

        auto buttonSubmit = Button(
            "提交", std::bind(&PlayPageBase::submit, this), option);
        buttonSubmit |= Maybe([&] { return m_state == State::input; });

        auto buttonRetry = Button(
            "重试本关", std::bind(&PlayPageBase::retry, this), option);
        buttonRetry |= Maybe([&] { return m_state == State::fail; });

        auto buttonBack = Button(
            "退出", [&] {
                m_socket.write("exit\n");
                *m_exit = true;
                switchPage(MainPage(m_ctx));
            },
            option);

        m_buttons = Container::Horizontal({buttonSubmit,
                                           buttonRetry,
                                           buttonBack});

        Add(Container::Vertical({m_input, m_buttons}));

        start();
    }

  private:
    Element Render() override {
        auto header = hbox({text(" " + std::to_string(m_ctx.user.level) + "级"
                                 + " 经验" + std::to_string(m_ctx.user.exp)
                                 + "/" + std::to_string(m_ctx.user.expForNextLevel) + " ["),
                            gauge((float)m_ctx.user.exp / m_ctx.user.expForNextLevel),
                            text("] 共通过" + std::to_string(m_ctx.user.levelPassed) + "个关卡 ")});

        Element tip;
        Element content;
        if (m_state == State::show) {
            tip = text("");
            content = text(m_word);
        } else if (m_state == State::input) {
            tip = text("");
            content = m_input->Render() | size(WIDTH, GREATER_THAN, 1);
        } else {
            content = text(m_inputText);
            if (m_state == State::correct) {
                if (m_expGained) {
                    tip = text("通过本关，耗时 "
                               + std::to_string(m_duration / 10) + "." + std::to_string(m_duration % 10) + " 秒，获得 "
                               + std::to_string(m_expGained) + " 经验");
                } else {
                    tip = text("答案正确");
                }
                content |= color(Color::Green);

            } else if (m_state == State::fail) {
                tip = text("答案错误，正确答案为 " + m_word + " 你还有" + std::to_string(m_retry) + "次重试机会");
                content |= color(Color::Red);
            }
        }

        return vbox({
                   header,
                   text("第 " + std::to_string(m_level) + " 关，第 " + std::to_string(m_round) + "/" + std::to_string(m_totalRound) + " 轮") | hcenter,
                   filler(),
                   text(m_countdown ? "倒计时 " + std::to_string(m_countdown / 10) + "." + std::to_string(m_countdown % 10) + " 秒" : "请输入") | hcenter,
                   filler(),
                   tip | hcenter,
                   content | hcenter,
                   filler(),
                   m_buttons->Render() | hcenter,
                   filler(),
               })
             | size(WIDTH, EQUAL, 90)
             | size(HEIGHT, EQUAL, 30)
             | border;
    }

    bool OnEvent(Event event) override {
        if (event == Event::Custom) {
            if (m_countdown > 0) {
                m_countdown--;
                if (m_countdown == 0) {
                    m_state = State::input;
                    m_inputText.clear();
                    m_input->TakeFocus();
                }
            }
            return true;
        } else if (event.input() == "get_problem") {
            getProblem();
        } else {
            return PageBase::OnEvent(event);
        }
    }

    void getProblem() {
        auto is = m_socket.readStream();
        std::string line;
        std::getline(is, line);
        std::getline(is, line);
        m_word = line;
        is >> m_level >> m_round >> m_totalRound >> m_countdown;
        m_state = State::show;
        std::thread([timeLimit = m_countdown, &screen = m_ctx.screen, exit = m_exit] {
            for (int i = 0; i < timeLimit; i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (*exit) return;
                screen.PostEvent(Event::Custom);
            }
        }).detach();
    }

    void start() {
        m_socket.write("play\n");
        getProblem();
    }

    void retry() {
        if (m_retry > 0) {
            m_retry--;
            m_socket.write("retry\n");
            getProblem();
        } else {
            alert("重试次数已用完");
        }
    }

    void submit() {
        if (m_inputText.empty()) return;
        m_socket.write("submit\n"
                       + m_inputText + "\n");

        auto is = m_socket.readStream();
        std::string line;
        std::getline(is, line);
        int result;
        is >> result
            >> m_duration >> m_expGained >> m_retry
            >> m_ctx.user.level
            >> m_ctx.user.exp
            >> m_ctx.user.expForNextLevel
            >> m_ctx.user.levelPassed;

        if (result) {
            m_state = State::correct;
            std::thread([this] {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                m_ctx.screen.PostEvent(Event::Special("get_problem"));
            }).detach();

        } else {
            m_state = State::fail;
        }
    }

    enum class State {
        show,
        input,
        correct,
        fail
    };

    State m_state;
    std::string m_word;
    int m_countdown;
    int m_duration, m_expGained;
    int m_level, m_round, m_totalRound, m_retry;
    std::string m_inputText;
    Component m_input, m_buttons;
    std::shared_ptr<std::atomic_bool> m_exit = std::make_shared<std::atomic_bool>(false);
};

Component PlayPage(GlobalContext &ctx) {
    return Make<PlayPageBase>(ctx);
}
