#include "GlobalContext.h"
#include "ui.h"

using namespace ftxui;

class MainPageBase : public PageBase {
  public:
    MainPageBase(GlobalContext &ctx) : PageBase(ctx) {
        auto buttonPlay = Button(
            "开始游戏", [&] { switchPage(PlayPage(ctx));*m_exit = true; }, ButtonOption::Animated());

        auto buttonBattle = Button(
            "匹配双人对战", std::bind(&MainPageBase::match, this), ButtonOption::Animated());

        auto buttonMake = Button(
            "开始出题", [&] { switchPage(MakePage(ctx));*m_exit = true; }, ButtonOption::Animated());

        auto buttonRank = Button(
            "查看排行榜", [&] { switchPage(RankPage(ctx));*m_exit = true; }, ButtonOption::Animated());

        if (m_ctx.user.type == 1) {
            m_centerButtons = Container::Horizontal({buttonPlay, buttonBattle, buttonRank});
        } else {
            m_centerButtons = Container::Horizontal({buttonMake, buttonRank});
        }

        ButtonOption option = ButtonOption::Ascii();
        auto buttonLogout = Button(
            "登出", [&] {
                m_socket.write("logout\n");
                m_ctx.router->switchPage(LoginPage(m_ctx));
            },
            option);

        auto buttonExit = Button(
            "退出", [&] { m_ctx.screen.Exit(); },
            option);

        m_bottomButtons = Container::Horizontal({buttonLogout, buttonExit});

        auto matchModal = Button(
                              "取消", [&] {
            m_socket.write("stop_match\n");
            m_matching = false; }, ButtonOption::Ascii())
                        | Renderer([&](Element inner) {
                              return vbox({text("正在为您匹配势均力敌的对手...") | hcenter,
                                           inner | hcenter})
                                   | border;
                          });

        auto content = Renderer(Container::Vertical({m_centerButtons, m_bottomButtons}), [&]() {
            Element header;
            if (m_ctx.user.type == 1) {
                header = hbox({text(" " + std::to_string(m_ctx.user.level) + "级"
                                    + " 经验" + std::to_string(m_ctx.user.exp)
                                    + "/" + std::to_string(m_ctx.user.expForNextLevel) + " ["),
                               gauge((float)m_ctx.user.exp / m_ctx.user.expForNextLevel),
                               text("] 共通过" + std::to_string(m_ctx.user.levelPassed) + "个关卡 ")});
            } else if (m_ctx.user.type == 2) {
                std::string info = " " + std::to_string(m_ctx.user.level) + "级 出题数"
                                 + std::to_string(m_ctx.user.madeNum)
                                 + "/" + std::to_string(m_ctx.user.madeNumForNextLevel) + " ";
                header = hbox({text(info)});
            }

            std::string welcome;
            welcome = (m_ctx.user.type == 1 ? "欢迎闯关者 " : "欢迎出题者 ") + m_ctx.user.name;
            return vbox({header,
                         filler(),
                         vbox({text(welcome) | hcenter, m_centerButtons->Render()}) | center | flex,
                         m_bottomButtons->Render() | hcenter,
                         filler()})
                 | size(WIDTH, EQUAL, 90)
                 | size(HEIGHT, EQUAL, 30)
                 | border;
        });

        Add(content | Modal(matchModal, &m_matching));

        std::thread([&screen = m_ctx.screen, exit = m_exit] {
            while (!*exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                screen.PostEvent(Event::Custom);
            }
        }).detach();
    }

  private:
    bool OnEvent(Event event) override {
        if (event == Event::Custom) {
            if (m_matching) {
                m_socket.write("poll_match\n");
                auto is = m_socket.readStream();
                std::string s;
                std::getline(is, s);
                std::getline(is, s);
                if (s == "1") {
                    m_matching = false;
                    *m_exit = true;
                    m_ctx.router->switchPage(BattlePage(m_ctx));
                }
            }
            return true;
        } else {
            return PageBase::OnEvent(event);
        }
    }

    void match() {
        m_socket.write("start_match\n");
        m_matching = true;
    }

    Component m_bottomButtons, m_centerButtons;
    std::shared_ptr<std::atomic_bool> m_exit = std::make_shared<std::atomic_bool>(false);
    bool m_matching = false;
};

Component MainPage(GlobalContext &ctx) {
    return Make<MainPageBase>(ctx);
}
