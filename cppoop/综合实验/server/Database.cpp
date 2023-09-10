#include "Database.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

Database db;

Database::Database() {
    std::random_device rd;
    m_randomEngine.seed(rd());
}

UserPtr Database::getUserByName(const std::string &name) {
    auto result = m_users.find(name);
    if (result == m_users.end()) {
        return nullptr;
    } else {
        return result->second;
    }
}

bool Database::addUser(UserPtr user) {
    if (m_users.find(user->getName()) != m_users.end()) {
        return false;
    } else {
        m_users.emplace(std::make_pair(user->getName(), user));
        m_unsaved = true;
        return true;
    }
}

bool Database::updateUser(UserPtr user) {
    auto t = m_users.find(user->getName());
    if (t == m_users.end()) {
        return false;
    } else {
        t->second = user;
        m_unsaved = true;
        return true;
    }
}

std::string Database::getUserListForClient() {
    std::stringstream os;
    os << "userlist_res\n";
    os << m_users.size() << "\n";
    for (const auto &[_, user] : m_users) {
        if (user->getType() == UserType::challenger) {
            os << "1\n";
            os << user->getName() << "\n";
            auto challenger = std::dynamic_pointer_cast<Challenger>(user);
            os << challenger->getLevel() << " "
               << challenger->getExp() << " "
               << challenger->getExpForNextLevel() << " "
               << challenger->getLevelPassed() << "\n";
        } else if (user->getType() == UserType::author) {
            os << "2\n";
            os << user->getName() << "\n";
            auto author = std::dynamic_pointer_cast<Author>(user);
            os << author->getLevel() << " "
               << author->getMadeNum() << " "
               << author->getMadeNumForNextLevel() << "\n";
        }
    }
    return os.str();
}

bool Database::addProblem(const Problem &problem) {
    if (m_wordSet.find(problem.word()) != m_wordSet.end()) {
        return false;
    }
    int index = m_problems.size();
    m_problems.push_back(problem);
    int length = problem.length();
    if (length >= m_problemIndexByLength.size()) {
        m_problemIndexByLength.resize(length + 1);
    }
    m_problemIndexByLength[length].push_back(index);
    m_wordSet.insert(problem.word());
    m_unsaved = true;
    return true;
}

Problem Database::getRandomProblem(int minLength, int maxLength) {
    minLength = std::min(minLength, static_cast<int>(m_problemIndexByLength.size()) - 1);
    maxLength = std::min(maxLength, static_cast<int>(m_problemIndexByLength.size()) - 1);
    int problemCount = 0;
    for (int i = minLength; i <= maxLength; i++) {
        problemCount += (int)m_problemIndexByLength[i].size();
    }
    if (problemCount == 0) {
        return Problem("");
    }
    std::uniform_int_distribution<> distrib(0, problemCount);
    int index = distrib(m_randomEngine);
    for (int i = minLength; i <= maxLength; i++) {
        if (index < m_problemIndexByLength[i].size()) {
            return m_problems[m_problemIndexByLength[i][index]];
        } else {
            index -= (int)m_problemIndexByLength[i].size();
        }
    }
    return Problem("");
}

void Database::save() {
    std::ofstream os("users.tsv");
    for (const auto &[_, user] : m_users) {
        os << user->serialize() << "\n";
    }
    std::cout << "saved " + std::to_string(m_users.size()) + " user(s)" << std::endl;

    os = std::ofstream("problems.tsv");
    for (const auto &problem : m_problems) {
        os << problem.serialize() << "\n";
    }
    std::cout << "saved " + std::to_string(m_problems.size()) + " problems(s)" << std::endl;

    m_unsaved = false;
}

void Database::load() {
    m_users.clear();
    std::ifstream is("users.tsv");
    if (is) {
        std::string line;
        while (std::getline(is, line)) {
            auto user = User::deserialize(line);
            m_users.emplace(std::make_pair(user->getName(), user));
        }
    }
    std::cout << "loaded " + std::to_string(m_users.size()) + " user(s)" << std::endl;

    m_problems.clear();
    m_problemIndexByLength.clear();
    m_wordSet.clear();
    is = std::ifstream("problems.tsv");
    if (is) {
        std::string line;
        while (std::getline(is, line)) {
            auto problem = Problem::deserialize(line);
            addProblem(problem);
        }
    }
    std::cout << "loaded " + std::to_string(m_problems.size()) + " problem(s)" << std::endl;
}

bool Database::unsaved() {
    return m_unsaved;
}
