#pragma once
#include "Router.h"
#include "Socket.h"
#include "ftxui/component/screen_interactive.hpp"

struct UserData {
    std::string name;
    int type;
    int level;
    int exp;
    int expForNextLevel;
    int levelPassed;
    int madeNum;
    int madeNumForNextLevel;
};

struct GlobalContext {
    GlobalContext(ftxui::ScreenInteractive &screen, Socket &socket, std::shared_ptr<RouterBase> router)
        : screen(screen), socket(socket), router(router) {}
    ftxui::ScreenInteractive &screen;
    Socket &socket;
    std::shared_ptr<RouterBase> router;
    UserData user;
};