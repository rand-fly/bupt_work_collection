#include "GlobalContext.h"
#include "ui.h"
#include <thread>

using namespace ftxui;

class BattlePageBase : public PageBase {
  public:
    BattlePageBase(GlobalContext &ctx) : PageBase(ctx) {
        InputOption inputOption;
        inputOption.on_enter = std::bind(&BattlePageBase::submit, this);
        m_input = Input(&m_inputText, "输入单词", inputOption);

        m_input |= Maybe([&] { return m_state == State::input; });

        ButtonOption option = ButtonOption::Ascii();

        auto buttonSubmit = Button(
            "提交", std::bind(&BattlePageBase::submit, this), option);
        buttonSubmit |= Maybe([&] { return m_state == State::input; });

        auto buttonBack = Button(
            "退出", [&] {
                m_socket.write("exit\n");
                *m_exit = true;
                switchPage(MainPage(m_ctx));
            },
            option);

        m_buttons = Container::Horizontal({buttonSubmit, buttonBack});

        Add(Container::Vertical({m_input, m_buttons}));

        m_socket.write("battle_ready\n");

        getProblem();

        std::thread([&screen = m_ctx.screen, exit = m_exit] {
            while (!*exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                screen.PostEvent(Event::Custom);
            }
        }).detach();
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
                tip = text("抢答成功，获得 " + std::to_string(m_expGained) + " 经验");
                content |= color(Color::Green);
            } else if (m_state == State::incorrect) {
                tip = text("答案错误，正确答案为 " + m_word + "，失去 " + std::to_string(-m_expGained) + " 经验");
                content |= color(Color::Red);
            } else if (m_state == State::opponentCorrect) {
                tip = text("对方抢答成功，失去 " + std::to_string(-m_expGained) + " 经验");
            } else if (m_state == State::opponentIncorrect) {
                tip = text("对方抢答失败，获得 " + std::to_string(m_expGained) + " 经验");
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
            if (m_polling) {
                m_socket.write("poll_result\n");
                auto is = m_socket.readStream();
                std::string line;
                std::getline(is, line);
                std::cerr << line << '\n';
                if (line == "battle_result") {
                    int result;
                    is >> result >> m_expGained
                        >> m_ctx.user.level
                        >> m_ctx.user.exp
                        >> m_ctx.user.expForNextLevel
                        >> m_ctx.user.levelPassed;
                    if (result == 0) {
                        m_state = State::incorrect;
                    } else if (result == 1) {
                        m_state = State::correct;
                    } else if (result == 2) {
                        m_state = State::opponentIncorrect;
                    } else if (result == 3) {
                        m_state = State::opponentCorrect;
                    } else if (result == 4) {
                        alert("对手已离开");
                        *m_exit = true;
                        switchPage(MainPage(m_ctx));
                        return true;
                    }
                    m_polling = false;
                    if (m_round < m_totalRound) {
                        std::thread([this] {
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            m_ctx.screen.PostEvent(Event::Special("get_problem"));
                        }).detach();
                    } else {
                        alert("对战结束");
                    }
                }
            }
            return true;
        } else if (event.input() == "get_problem") {
            m_polling = true;
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
    }

    void submit() {
        if (m_inputText.empty()) return;
        m_socket.write("submit\n"
                       + m_inputText + "\n");
    }

    enum class State {
        show,
        input,
        correct,
        incorrect,
        opponentCorrect,
        opponentIncorrect
    };

    State m_state;
    std::string m_word;
    int m_countdown;
    int m_expGained;
    int m_level, m_round, m_totalRound;
    std::string m_inputText;
    Component m_input, m_buttons;
    std::atomic_bool m_polling = true;
    std::shared_ptr<std::atomic_bool> m_exit = std::make_shared<std::atomic_bool>(false);
};

Component BattlePage(GlobalContext &ctx) {
    return Make<BattlePageBase>(ctx);
}
