#include "LL1Table.h"
#include <iomanip>
#include <sstream>

namespace LL1_parser {
bool LL1Table::tryInsert(const pair<Symbol, Symbol> &key, const Phrase &value) {
    if (table_.find(key) != table_.end()) {
        return false;
    }
    table_.insert({key, value});
    nonTerminals_.insert(key.first);
    terminals_.insert(key.second);
    return true;
}

Symbol LL1Table::getStart() const { return start_; }

void LL1Table::print() const {
    std::cout << std::setw(5) << std::left << "";
    for (const auto &col : terminals_) {
        std::cout << std::setw(12) << std::left << col;
    }
    std::cout << std::endl;
    for (const auto &row : nonTerminals_) {
        std::cout << std::setw(5) << std::left << row;
        for (const auto &col : terminals_) {
            auto iter = table_.find({row, col});
            if (iter != table_.end()) {
                std::stringstream ss;
                ss << row << "->" << iter->second;
                std::cout << std::setw(12) << std::left << ss.str();
            } else {
                std::cout << std::setw(12) << std::left << "";
            }
        }
        std::cout << std::endl;
    }
}

bool LL1Table::parse(std::function<Symbol()> getNext) const {
    stack<Symbol> st;
    int step = 0;
    st.push(start_);
    Symbol next = getNext();
    while (!st.empty()) {
        auto top = st.top();
        st.pop();
        if (top.isTerminal()) {
            if (top == next) {
                next = getNext();
                continue;
            } else {
                return false;
            }
        } else {
            if (table_.find({top, next}) == table_.end()) {
                return false;
            }
            auto right = table_.at({top, next});
            std::cout << "(" << ++step << ") " << top << " → " << right << std::endl;
            for (auto it = right.rbegin(); it != right.rend(); ++it) {
                st.push(*it);
            }
        }
    }
    return next == Symbol::end;
}
} // namespace LL1_parser