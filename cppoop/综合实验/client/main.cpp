#include "GlobalContext.h"
#include "Router.h"
#include "asio.hpp"
#include "ftxui/component/component.hpp"          // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp" // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for text, hbox, separator, Element, operator|, vbox, border
#include "protocol.h"
#include "ui.h"
#include <iostream>
#include <scroller.hpp>
#include <sstream>
#include <string> // for char_traits, operator+, string, basic_string

using namespace ftxui;
using asio::ip::tcp;

const char *port = "1764"; // yh's number

std::stringstream gout;
bool showDebug = false;

int main(int argc, const char *argv[]) {
    std::cerr.rdbuf(gout.rdbuf());
    auto screen = ftxui::ScreenInteractive::Fullscreen();

    Socket socket;
    auto router = Router();
    GlobalContext ctx(screen, socket, router);

    router->switchPage(ConnectPage(ctx));

    auto debugOutput = Renderer([&] {
        auto debugLines = Elements();
        std::string line;
        std::stringstream ss(gout.str());
        while (std::getline(ss, line), ss.good()) {
            debugLines.push_back(paragraph(line));
        }
        return vbox(debugLines);
    });

    auto scroller = Scroller(debugOutput);
    auto component = Container::Vertical({router,
                                          scroller | Maybe([&] { return showDebug; })});

    auto renderer = Renderer(component, [&] {
        if (showDebug) {
            return vbox({router->Render() | center | flex,
                         window(text("调试输出"),
                                debugOutput->Render() | size(HEIGHT, EQUAL, 10))});
        } else {
            return router->Render() | center;
        }
    });
    renderer |= CatchEvent([&](Event event) {
        if (event == Event::F1) {
            showDebug = !showDebug;
        }
        return false;
    });

    screen.Loop(renderer);
}
