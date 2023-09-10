#pragma once
#include "GlobalContext.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"

class PageBase : public ftxui::ComponentBase {
  public:
    PageBase(GlobalContext &ctx) : m_ctx(ctx) {}

  protected:
    GlobalContext &m_ctx;
    Socket &m_socket = m_ctx.socket;

    void switchPage(ftxui::Component page) {
        m_ctx.router->switchPage(page);
    }

    void alert(const std::string &msg) {
        m_ctx.router->alert(msg);
    }
};

ftxui::Component ConnectPage(GlobalContext &ctx);
ftxui::Component LoginPage(GlobalContext &ctx);
ftxui::Component SignupPage(GlobalContext &ctx);
ftxui::Component MainPage(GlobalContext &ctx);
ftxui::Component PlayPage(GlobalContext &ctx);
ftxui::Component MakePage(GlobalContext &ctx);
ftxui::Component RankPage(GlobalContext &ctx);
ftxui::Component BattlePage(GlobalContext &ctx);
