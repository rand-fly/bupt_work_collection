#pragma once
#include "using.h"
#include <iostream>

namespace LL1_parser {
class Symbol {
  public:
    Symbol(const string &name, bool isTerminal) : name_(name), isTerminal_(isTerminal) {}
    Symbol(const char *name, bool isTerminal) : name_(name), isTerminal_(isTerminal) {}
    Symbol(const string &name) : name_(name) {
        if (name_.size() >= 1 && isupper(name_[0])) {
            isTerminal_ = false;
        } else {
            isTerminal_ = true;
        }
    }
    Symbol(const char *name) : name_(name) {
        if (name_.size() >= 1 && isupper(name_[0])) {
            isTerminal_ = false;
        } else {
            isTerminal_ = true;
        }
    }

    string name() const { return name_; }
    bool isTerminal() const { return isTerminal_; }
    Symbol toAlternate() const { return Symbol(name_ + "'", false); }

    bool operator==(const Symbol &rhs) const { return name_ == rhs.name_; }
    bool operator!=(const Symbol &rhs) const { return name_ != rhs.name_; }
    bool operator<(const Symbol &rhs) const { return name_ < rhs.name_; }
    bool operator>(const Symbol &rhs) const { return name_ > rhs.name_; }
    bool operator<=(const Symbol &rhs) const { return name_ <= rhs.name_; }
    bool operator>=(const Symbol &rhs) const { return name_ >= rhs.name_; }

    friend std::ostream &operator<<(std::ostream &os, const Symbol &symbol) {
        os << symbol.name_;
        return os;
    }

    // 下面两个特殊符号仅用于first集和follow集，在产生式中表示ε应使用空的Phrase
    static const Symbol epsilon;
    static const Symbol end;

  private:
    string name_;
    bool isTerminal_;
};

inline const Symbol Symbol::epsilon = Symbol("ε", true);
inline const Symbol Symbol::end = Symbol("$", true);

using Phrase = vector<Symbol>;

inline std::ostream &operator<<(std::ostream &os, const Phrase &phrase) {
    if (phrase.empty()) {
        os << "ε";
    } else {
        for (const auto &symbol : phrase) {
            os << symbol;
        }
    }
    return os;
}

} // namespace LL1_parser
