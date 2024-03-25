#pragma once

#include <string>

struct Token {
    Token(std::string type, std::string val, int line, int col)
        : type(type), val(val), line(line), col(col) {}

    void print() {
        printf("%d:%d: <%s, %s>\n", line, col, type.c_str(), val.c_str());
    }

    std::string type;
    std::string val;
    int line;
    int col;
};
