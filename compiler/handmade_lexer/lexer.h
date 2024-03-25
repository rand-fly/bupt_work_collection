#pragma once

#include "token.h"
#include <map>

class Lexer {
  public:
    Lexer(std::istream &in, bool colored = true) : in_(in), colored_(colored) {}

    Token getToken();
    void printStat();

  private:
    std::istream &in_;
    std::string token_;
    int line_ = 1;
    int row_ = 1;
    int beginLine_;
    int beginRow_;
    bool colored_;
    std::map<std::string, std::map<std::string, int>> stat_;
    int charCount_ = 0;
    int lineCount_ = 0;

    int peek();
    int get();
    int pass();

    Token makeToken(const std::string &type);

    Token getIdentifierToken();
    Token getStringToken();
    Token getCharToken();
    Token getNumericToken(bool readDot);

    bool getEscapeSequence();
    bool getIntSuffix();
    bool getFloatSuffix();
    bool getUniversalCharacterName();
  
    void error(const std::string &msg);
    void warning(const std::string &msg);
};
