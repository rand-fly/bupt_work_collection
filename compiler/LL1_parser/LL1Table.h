#pragma once
#include "Symbol.h"
#include "using.h"
#include <functional>

namespace LL1_parser {

class LL1Table {
  public:
    LL1Table(const Symbol &start) : start_(start) {}
    bool tryInsert(const pair<Symbol, Symbol> &key, const Phrase &value);
    Symbol getStart() const;
    void print() const;
    bool parse(std::function<Symbol()> getNext) const;

  private:
    map<pair<Symbol, Symbol>, Phrase> table_;
    Symbol start_;
    set<Symbol> nonTerminals_;
    set<Symbol> terminals_;
};

} // namespace LL1_parser
