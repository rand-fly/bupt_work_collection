#ifdef __WIN32
#include <windows.h>
#endif
#include "Grammar.h"
#include "LR1Table.h"
#include "Symbol.h"
#include "using.h"
#include <io.h>

using namespace LR_parser;

Symbol lexer();

int main() {
#ifdef __WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    Grammar grammar;
    grammar.addRule("E", {"E", "+", "T"});
    grammar.addRule("E", {"E", "-", "T"});
    grammar.addRule("E", {"T"});
    grammar.addRule("T", {"T", "*", "F"});
    grammar.addRule("T", {"T", "/", "F"});
    grammar.addRule("T", {"F"});
    grammar.addRule("F", {"(", "E", ")"});
    grammar.addRule("F", {"num"});
    grammar.setStart("E");

    grammar.print();
    std::cout << std::endl;

    grammar.exGrammar();
    grammar.print();
    std::cout << std::endl;

    auto table = grammar.buildLR1Table();
    table.print();
    std::cout << std::endl;

    if (table.parse(lexer)) {
        std::cout << "Accept!" << std::endl;
    } else {
        std::cout << "Reject!" << std::endl;
    }

    return 0;
}

Symbol lexer() {
    for (;;) {
        int c = getchar();
        if (isdigit(c)) {
            while (isdigit(c = getchar()))
                ;
            ungetc(c, stdin);
            return Symbol("num");
        }
        if (isspace(c)) {
            continue;
        }
        switch (c) {
        case EOF:
        case '$':
            return Symbol::end;
        case '+':
            return Symbol("+");
        case '-':
            return Symbol("-");
        case '*':
            return Symbol("*");
        case '/':
            return Symbol("/");
        case '(':
            return Symbol("(");
        case ')':
            return Symbol(")");
        default:
            std::cout << "unexpected character: " << (char)c << std::endl;
            exit(0);
        }
    }
}
