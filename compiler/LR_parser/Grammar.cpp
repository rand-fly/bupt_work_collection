#include "Grammar.h"
#include <algorithm>
#include <cassert>
#include <iostream>

namespace LR_parser {

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

void Grammar::exGrammar() {
    Symbol oldStart = start_;
    Symbol newStart = start_.toAlternate();
    rules_[newStart].push_back({oldStart});
    start_ = newStart;

    int idxCnt = 0;
    for (auto &[left, rightList] : rules_) {
        for (auto &right : rightList) {
            right.idx = idxCnt++;
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

set<Item> Grammar::closure(const set<Item> &items) const {
    set<Item> newItems = items;
    bool changed = false;
    do {
        changed = false;
        for (const auto &item : newItems) {
            if (item.dotPos == (ssize_t)item.right.size()) {
                continue;
            }
            Symbol nextSymbol = item.right[item.dotPos];
            if (nextSymbol.isTerminal()) {
                continue;
            }
            for (const auto &right : rules_.at(nextSymbol)) {
                Item newItem{nextSymbol, right, 0, right.idx};
                if (find(newItems.begin(), newItems.end(), newItem) == newItems.end()) {
                    newItems.insert(newItem);
                    changed = true;
                }
            }
        }
    } while (changed);
    return newItems;
}

LR1Table Grammar::buildLR1Table() const {
    auto followSets = getAllFollowSets();

    vector<pair<Symbol, Phrase>> rules;
    for (const auto &[left, rightList] : rules_) {
        for (const auto &right : rightList) {
            rules.push_back({left, right});
        }
    }
    LR1Table table(rules);
    vector<ItemSet> itemSets;
    map<set<Item>, int> items2Id;
    queue<int> q;
    auto startItem = Item{start_, rules_.at(start_)[0], 0, rules_.at(start_)[0].idx};
    auto startItemSet = closure({startItem});
    itemSets.push_back({startItemSet, {}});
    items2Id[startItemSet] = 0;
    q.push(0);
    while (!q.empty()) {
        int id = q.front();
        q.pop();
        map<Symbol, set<Item>> nextKernelItemSets;
        for (const Item &item : itemSets[id].items) {
            if (item.dotPos == (ssize_t)item.right.size()) {
                for (const auto &follow : followSets[item.left]) {
                    if (item.left == start_) {
                        if (!table.tryInsertAction({id, follow}, {Action::ACCEPT, 0})) {
                            printf("Not SLR(1) grammar!\n");
                            exit(0);
                        }
                    } else {
                        if (!table.tryInsertAction({id, follow}, {Action::REDUCE, item.id})) {
                            printf("Not SLR(1) grammar!\n");
                            exit(0);
                        }
                    }
                }
            } else {
                Symbol nextSymbol = item.right[item.dotPos];
                Item newItem = {item.left, item.right, item.dotPos + 1, item.id};
                nextKernelItemSets[nextSymbol].insert(newItem);
            }
        }
        for (const auto &[symbol, itemSet] : nextKernelItemSets) {
            auto nextItemSet = closure(itemSet);
            int nextId;
            if (items2Id.find(nextItemSet) == items2Id.end()) {
                items2Id[nextItemSet] = itemSets.size();
                itemSets.push_back({nextItemSet, {}});
                q.push(itemSets.size() - 1);
                nextId = itemSets.size() - 1;
            } else {
                nextId = items2Id[nextItemSet];
            }
            itemSets[id].to[symbol] = nextId;
            if (symbol.isTerminal()) {
                if (!table.tryInsertAction({id, symbol}, {Action::SHIFT, nextId})) {
                    printf("Not SLR(1) grammar!\n");
                    exit(0);
                }
            } else {
                table.insertGoto({id, symbol}, nextId);
            }
        }
    }
    for (int i = 0; i < (ssize_t)itemSets.size(); i++) {
        printf("I%d[label=\"I%d:\\n", i, i);
        for (const auto &item : itemSets[i].items) {
            std::cout << item << "\\n";
        }
        printf("\"]\n");
        for (const auto &[symbol, dest] : itemSets[i].to) {
            printf("I%d->I%d[label=\"%s\"]\n", i, dest, symbol.name().c_str());
        }
    }
    return table;
}

} // namespace LR_parser
