#pragma once
#include "LR1Table.h"
#include "Symbol.h"
#include "using.h"

namespace LR_parser {
struct Item {
    Symbol left;
    Phrase right;
    int dotPos;
    int id;
    friend bool operator==(const Item &a, const Item &b) {
        return a.left == b.left && a.right == b.right && a.dotPos == b.dotPos;
    }
    friend bool operator<(const Item &a, const Item &b) {
        if (a.left != b.left) {
            return a.left < b.left;
        }
        if (a.right != b.right) {
            return a.right < b.right;
        }
        return a.dotPos < b.dotPos;
    }
    friend std::ostream &operator<<(std::ostream &os, const Item &item) {
        os << item.left << "→";
        if (item.right.empty()) {
            os << "ε";
        } else {
            for (int i = 0; i < (ssize_t)item.right.size(); ++i) {
                if (i == item.dotPos) {
                    os << "•";
                }
                os << item.right[i];
            }
            if (item.dotPos == (ssize_t)item.right.size()) {
                os << "•";
            }
        }
        return os;
    }
};
struct ItemSet {
    set<Item> items;
    map<Symbol, int> to;
};

class Grammar {
  public:
    void addRule(const Symbol &left, const Phrase &right);
    void setStart(const Symbol &start);
    void print() const;

    void exGrammar();

    set<Symbol> getFirstSet(const Phrase &phrase) const;
    map<Symbol, set<Symbol>> getAllFollowSets() const;

    set<Item> closure(const set<Item> &items) const;
    LR1Table buildLR1Table() const;

  private:
    map<Symbol, vector<Phrase>> rules_;
    Symbol start_{"S"};
    mutable map<Phrase, set<Symbol>> firstSetCache_;
};
} // namespace LR_parser
