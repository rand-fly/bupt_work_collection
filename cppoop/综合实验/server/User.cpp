#include "User.h"
#include "Database.h"
#include <sstream>
#include <vector>

using std::to_string, std::stoi;

std::string User::serialize() const {
    return to_string(static_cast<int>(getType())) + "\t"
         + m_name + "\t"
         + m_password + "\t"
         + to_string(m_level);
}

std::string Challenger::getInfo() const {
    std::stringstream ss;
    ss << getLevel() << " "
       << getExp() << " "
       << getExpForNextLevel() << " "
       << getLevelPassed() << "\n";
    return ss.str();
}

std::string Challenger::serialize() const {
    return to_string(static_cast<int>(getType())) + "\t"
         + m_name + "\t"
         + m_password + "\t"
         + to_string(m_level) + "\t"
         + to_string(m_exp) + "\t"
         + to_string(m_levelPassed);
}

int Challenger::getExpForNextLevel() const {
    return m_level * 50;
}

void Challenger::addExp(int exp) {
    m_exp += exp;
    if (m_exp < 0) m_exp = 0;
    while (m_exp >= getExpForNextLevel()) {
        m_exp -= getExpForNextLevel();
        m_level++;
    }
    db.m_unsaved = true;
}

void Challenger::passLevel() {
    m_levelPassed++;
    db.m_unsaved = true;
}

std::string Author::getInfo() const {
    std::stringstream ss;
    ss << getLevel() << " "
       << getMadeNum() << " "
       << getMadeNumForNextLevel() << "\n";
    return ss.str();
}

std::string Author::serialize() const {
    return to_string(static_cast<int>(getType())) + "\t"
         + m_name + "\t"
         + m_password + "\t"
         + to_string(m_level) + "\t"
         + to_string(m_madeNum);
}

int Author::getMadeNumForNextLevel() const {
    return m_level * (m_level + 1);
    db.m_unsaved = true;
}

void Author::addProblem() {
    m_madeNum++;
    if (m_madeNum >= getMadeNumForNextLevel()) {
        m_level++;
    }
    db.m_unsaved = true;
}

UserPtr User::deserialize(const std::string &str) {
    std::stringstream ss(str);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(ss, token, '\t')) {
        tokens.push_back(token);
    }
    if (tokens.size() < 4) {
        return nullptr;
    }
    UserType type = static_cast<UserType>(std::stoi(tokens[0]));
    if (type == UserType::base) {
        return std::make_shared<User>(tokens[1], tokens[2], stoi(tokens[3]));
    } else if (type == UserType::challenger) {
        return std::make_shared<Challenger>(tokens[1], tokens[2], stoi(tokens[3]), stoi(tokens[4]), stoi(tokens[5]));
    } else if (type == UserType::author) {
        return std::make_shared<Author>(tokens[1], tokens[2], stoi(tokens[3]), stoi(tokens[4]));
    }
    return nullptr;
}