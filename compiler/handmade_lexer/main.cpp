#include "lexer.h"
#include <cstring>
#include <fstream>
#include <io.h>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

void error(const std::string &msg, bool colored);
void initColor();

int main(int argc, char *argv[]) {
    bool colored = isatty(fileno(stderr));
    if (colored) initColor();

    std::ifstream fin;
    if (argc > 1) {
        fin.open(argv[1]);
        if (!fin) {
            error(strerror(errno), colored);
            return 1;
        }
    }
    std::istream &in = argc > 1 ? fin : std::cin;
    Lexer lexer(in, colored);
    for (;;) {
        Token token = lexer.getToken();
        if (token.type == "EOF") {
            break;
        } else {
            token.print();
        }
    }
    std::cout << std::endl;
    lexer.printStat();
    return 0;
}

void error(const std::string &msg, bool colored) {
    if (colored) std::cerr << "\033[1;31m";
    std::cerr << "error: ";
    if (colored) std::cerr << "\033[0m";
    std::cerr << msg << std::endl;
}

void initColor() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}