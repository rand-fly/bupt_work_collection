#pragma once
#include "Problem.h"
#include "User.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>

class Database {
  public:
    Database();

    UserPtr getUserByName(const std::string &name);
    bool addUser(UserPtr user);
    bool updateUser(UserPtr user);
    std::string getUserListForClient();

    bool addProblem(const Problem &problem);
    Problem getRandomProblem(int minLength, int maxLength);

    void save();
    void load();
    bool unsaved();

  private:
    friend class Challenger;
    friend class Author;


    std::unordered_map<std::string, UserPtr> m_users;
    std::vector<Problem> m_problems;
    std::vector<std::vector<int>> m_problemIndexByLength;
    std::unordered_set<std::string> m_wordSet;
    bool m_unsaved = false;
    
    std::default_random_engine m_randomEngine;
};

extern Database db;
