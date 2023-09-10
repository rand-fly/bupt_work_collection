#include "GlobalContext.h"
#include "asio.hpp"
#include "ui.h"
#include <iostream>

using namespace ftxui;
using asio::ip::tcp;

class ConnectPageBase : public PageBase {
  public:
    ConnectPageBase(GlobalContext &ctx) : PageBase(ctx) {
        InputOption option;
        option.on_enter = [&] {
            if (m_socket.connect(m_server)) {
                m_ctx.router->switchPage(LoginPage(ctx));
            } else {
                alert("连接失败");
            }
        };
        option.cursor_position = m_server.length();
        m_inputServer = Input(&m_server, "服务器地址", option);
        Add(m_inputServer);
    }

  private:
    virtual Element Render() override {
        return vbox({
                   text(" 输入服务器地址") | hcenter,
                   separator(),
                   m_inputServer->Render(),
               })
             | border
             | size(WIDTH, GREATER_THAN, 40);
    }

    std::string m_server = "127.0.0.1:1764";
    Component m_inputServer, m_serverBox;
};

Component ConnectPage(GlobalContext &ctx) {
    return Make<ConnectPageBase>(ctx);
}
