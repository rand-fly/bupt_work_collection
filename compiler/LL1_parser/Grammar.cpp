#include "Grammar.h"
#include <algorithm>
#include <cassert>
#include <iostream>

namespace LL1_parser {

void Grammar::addRule(const Symbol &left, const Phrase &right) {
    rules_[left].push_back(right);
    firstSetCache_.clear();
}

void Grammar::setStart(const Symbol &start) {
    start_ = start;
    firstSetCache_.clear();
}

void Grammar::print() const {
    for (const auto &[left, rightList] : rules_) {
        std::cout << left << " → ";
        bool first = true;
        for (const auto &right : rightList) {
            if (first) {
                first = false;
            } else {
                std::cout << "|";
            }
            std::cout << right;
        }
        std::cout << std::endl;
    }
}

void Grammar::solveLeftRecursion() {
    map<Symbol, vector<Phrase>> altRules;
    for (auto &[left, rightList] : rules_) {
        vector<Phrase> tempRightList;
        // 代入前面已经消除左递归的产生式
        for (const auto &right : rightList) {
            if (right.empty()) {
                continue;
            }
            Symbol first = right[0];
            if (!first.isTerminal() && first < left) {
                for (const auto &right2 : rules_[first]) {
                    auto newRight = right2;
                    newRight.insert(newRight.end(), right.begin() + 1, right.end());
                    tempRightList.push_back(newRight);
                }
            } else {
                tempRightList.push_back(right);
            }
        }
        // 消除左递归
        Symbol altLeft = left.toAlternate();
        vector<Phrase> newRightList, altRightList;
        for (auto &right : tempRightList) {
            if (right[0] == left) {
                right.erase(right.begin());
                right.push_back(altLeft);
                altRightList.push_back(std::move(right));
            } else {
                right.push_back(altLeft);
                newRightList.push_back(std::move(right));
            }
        }
        if (!altRightList.empty()) {
            altRightList.push_back({});
            rightList = newRightList;
            altRules[altLeft] = altRightList;
        }
    }
    rules_.insert(altRules.begin(), altRules.end());
    firstSetCache_.clear();
}

void Grammar::solveLeftCommonFactor() {
    queue<Symbol> solveQueue;
    for (const auto &[left, rightList] : rules_) {
        solveQueue.push(left);
    }
    while (!solveQueue.empty()) {
        Symbol left = solveQueue.front();
        solveQueue.pop();
        vector<Phrase> &rightList = rules_[left];
        map<Symbol, vector<Phrase>> rightWithFirstMap;
        for (const auto &right : rightList) {
            if (!right.empty()) {
                rightWithFirstMap[right[0]].push_back(right);
            }
        }
        Symbol altLeft = left;
        for (auto &[first, rightWithFirstList] : rightWithFirstMap) {
            if (rightWithFirstList.size() > 1) {
                altLeft = altLeft.toAlternate();
                auto &altLeftRightList = rules_[altLeft];
                for (auto &right : rightWithFirstList) {
                    altLeftRightList.emplace_back(right.begin() + 1, right.end());
                    rightList.erase(std::find(rightList.begin(), rightList.end(), right));
                }
                rightList.push_back({first, altLeft});
                solveQueue.push(altLeft);
            }
        }
    }
    firstSetCache_.clear();
}

set<Symbol> Grammar::getFirstSet(const Phrase &phrase) const {
    auto iter = firstSetCache_.find(phrase);
    if (iter != firstSetCache_.end()) {
        return iter->second;
    }
    firstSetCache_[phrase] = {}; // 避免无穷递归
    set<Symbol> firstSet;
    bool allHaveEpsilon = true;
    for (const auto &symbol : phrase) {
        if (symbol.isTerminal()) {
            firstSet.insert(symbol);
            allHaveEpsilon = false;
            break;
        } else {
            bool haveEpsilon = false;
            for (const auto &right : rules_.at(symbol)) {
                auto s = getFirstSet(right);
                if (s.find(Symbol::epsilon) != s.end()) {
                    s.erase(Symbol::epsilon);
                    haveEpsilon = true;
                }
                firstSet.merge(s);
            }
            if (!haveEpsilon) {
                allHaveEpsilon = false;
                break;
            }
        }
    }
    if (allHaveEpsilon) {
        firstSet.insert(Symbol::epsilon);
    }
    firstSetCache_[phrase] = firstSet;
    return firstSet;
}

map<Symbol, set<Symbol>> Grammar::getAllFollowSets() const {
    map<Symbol, set<Symbol>> followSets;
    followSets[start_].insert(Symbol::end);
    bool changed = false;
    do {
        changed = false;
        for (const auto &[left, rightList] : rules_) {
            for (const auto &right : rightList) {
                int len = right.size();
                for (int i = 0; i < len; i++) {
                    if (right[i].isTerminal()) {
                        continue;
                    }
                    auto oldFollowSet = followSets[right[i]];
                    Phrase beta(right.begin() + i + 1, right.end());
                    auto betaFirstSet = getFirstSet(beta);
                    if (betaFirstSet.find(Symbol::epsilon) != betaFirstSet.end()) {
                        betaFirstSet.erase(Symbol::epsilon);
                        followSets[right[i]].insert(followSets[left].begin(),
                                                    followSets[left].end());
                    }
                    followSets[right[i]].merge(betaFirstSet);
                    if (oldFollowSet != followSets[right[i]]) {
                        changed = true;
                    }
                }
            }
        }
    } while (changed);
    return followSets;
}

LL1Table Grammar::buildLL1Table() const {
    LL1Table table{start_};
    auto followSets = getAllFollowSets();
    for (const auto &[left, rightList] : rules_) {
        for (const auto &right : rightList) {
            auto firstSet = getFirstSet(right);
            for (const auto &symbol : firstSet) {
                if (symbol == Symbol::epsilon) {
                    for (const auto &symbol2 : followSets.at(left)) {
                        if (!table.tryInsert({left, symbol2}, right)) {
                            std::cout << "Not LL(1) grammar!" << std::endl;
                            exit(0);
                        }
                    }
                } else {
                    if (!table.tryInsert({left, symbol}, right)) {
                        std::cout << "Not LL(1) grammar!" << std::endl;
                        exit(0);
                    }
                }
            }
        }
    }
    return table;
}

} // namespace LL1_parser
