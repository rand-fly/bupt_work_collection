#include "LR1Table.h"
#include <iomanip>
#include <sstream>

namespace LR_parser {
bool LR1Table::tryInsertAction(const pair<int, Symbol> &key, const Action &value) {
    if (action_.find(key) != action_.end()) {
        return false;
    }
    action_.insert({key, value});
    terminals_.insert(key.second);
    stateNum_ = std::max(stateNum_, key.first + 1);
    return true;
}

void LR1Table::insertGoto(const pair<int, Symbol> &key, int value) {
    goto_.insert({key, value});
    nonTerminals_.insert(key.second);
    stateNum_ = std::max(stateNum_, key.first + 1);
}

void LR1Table::print() const {
    std::cout << std::setw(6 + terminals_.size() * 3 - 12) << std::left << "";
    std::cout << std::setw(terminals_.size() * 3 + 12) << std::left << "action";
    std::cout << std::setw(nonTerminals_.size() * 3 - 12) << std::left << "";
    std::cout << std::setw(nonTerminals_.size() * 3 + 12) << std::left << "goto";
    std::cout << std::endl;
    std::cout << std::setw(6) << std::left << "";
    for (const auto &col : terminals_) {
        std::cout << std::setw(6) << std::left << col;
    }
    for (const auto &col : nonTerminals_) {
        std::cout << std::setw(6) << std::left << col;
    }
    std::cout << std::endl;
    for (int i = 0; i < stateNum_; ++i) {
        std::cout << std::setw(6) << std::left << i;
        for (const auto &col : terminals_) {
            auto iter = action_.find({i, col});
            if (iter != action_.end()) {
                std::stringstream ss;
                if (iter->second.type == Action::SHIFT) {
                    ss << "S" << iter->second.state;
                } else if (iter->second.type == Action::REDUCE) {
                    ss << "R" << iter->second.rule;
                } else if (iter->second.type == Action::ACCEPT) {
                    ss << "ACC";
                }
                std::cout << std::setw(6) << std::left << ss.str();
            } else {
                std::cout << std::setw(6) << std::left << "";
            }
        }
        for (const auto &col : nonTerminals_) {
            auto iter = goto_.find({i, col});
            if (iter != goto_.end()) {
                std::cout << std::setw(6) << std::left << iter->second;
            } else {
                std::cout << std::setw(6) << std::left << "";
            }
        }
        std::cout << std::endl;
    }
}

bool LR1Table::parse(std::function<Symbol()> getNext) const {
    stack<int> st;
    st.push(0);
    int step = 0;
    Symbol next = getNext();
    while (!st.empty()) {
        auto top = st.top();
        auto it = action_.find({top, next});
        if (it == action_.end()) {
            return false;
        }
        auto action = it->second;
        if (action.type == Action::ACCEPT) {
            break;
        } else if (action.type == Action::SHIFT) {
            st.push(action.state);
            next = getNext();
        } else if (action.type == Action::REDUCE) {
            std::cout << "(" << ++step << ") ";
            auto rule = rules_[action.rule];
            std::cout << rule.first << " → " << rule.second << std::endl;
            for (int i = 0; i < (ssize_t)rule.second.size(); ++i) {
                st.pop();
            }
            st.push(goto_.at({st.top(), rule.first}));
        }
    }
    return next == Symbol::end;
}
} // namespace LR_parser