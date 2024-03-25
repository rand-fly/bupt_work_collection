#pragma once
#include "LL1Table.h"
#include "Symbol.h"
#include "using.h"

namespace LL1_parser {

class Grammar {
  public:
    void addRule(const Symbol &left, const Phrase &right);
    void setStart(const Symbol &start);
    void print() const;

    void solveLeftRecursion();
    void solveLeftCommonFactor();

    set<Symbol> getFirstSet(const Phrase &phrase) const;
    map<Symbol, set<Symbol>> getAllFollowSets() const;
    LL1Table buildLL1Table() const;

  private:
    map<Symbol, vector<Phrase>> rules_;    
    Symbol start_{"S"};
    mutable map<Phrase, set<Symbol>> firstSetCache_;
};
} // namespace LL1_parser
