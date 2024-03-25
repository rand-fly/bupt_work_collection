#pragma once
#include "Symbol.h"
#include "using.h"
#include <functional>

namespace LR_parser {

struct Action {
    enum Type { SHIFT, REDUCE, ACCEPT } type;
    union {
        int state;
        int rule;
    };
};

class LR1Table {
  public:
    LR1Table(const vector<pair<Symbol, Phrase>> &rules) : rules_(rules) {}
    bool tryInsertAction(const pair<int, Symbol> &key, const Action &value);
    void insertGoto(const pair<int, Symbol> &key, int value);
    void print() const;
    bool parse(std::function<Symbol()> getNext) const;

  private:
    vector<pair<Symbol, Phrase>> rules_;
    map<pair<int, Symbol>, Action> action_;
    map<pair<int, Symbol>, int> goto_;
    set<Symbol> nonTerminals_;
    set<Symbol> terminals_;
    int stateNum_ = 0;
};

} // namespace LR_parser
