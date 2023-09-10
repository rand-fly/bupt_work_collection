#pragma once
#include "Database.h"
#include <functional>
#include <iostream>
#include <sstream>

using std::string, std::getline, std::cout, std::to_string;

class Battle {
  public:
    Battle(Challenger &challenger1,
           Challenger &challenger2,
           std::function<void(const std::string &s)> async_write1,
           std::function<void(const std::string &s)> async_write2);

    bool handle(int side, std::string msg);

    void end(int side);

  private:
    void makeProblem();

    Challenger &m_challenger1, &m_challenger2;
    std::function<void(const std::string &s)> m_async_write1, m_async_write2;
    bool m_ready1 = false, m_ready2 = false;

    std::string m_result1, m_result2;
    bool m_ended = false;

    int m_level, m_round;
    Problem m_problem{""};
    std::string m_problemMsg;
};