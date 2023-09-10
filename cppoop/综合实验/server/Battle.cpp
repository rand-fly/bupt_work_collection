#include "Battle.h"

Battle::Battle(Challenger &challenger1,
               Challenger &challenger2,
               std::function<void(const std::string &s)> async_write1,
               std::function<void(const std::string &s)> async_write2)
    : m_challenger1(challenger1), m_challenger2(challenger2),
      m_async_write1(async_write1), m_async_write2(async_write2) {
    m_level = 8;
    m_round = 1;
}

bool Battle::handle(int side, std::string msg) {
    std::istringstream is(msg);
    auto &challengerNow = side == 1 ? m_challenger1 : m_challenger2;
    auto &challengerOppose = side == 1 ? m_challenger2 : m_challenger1;
    auto &asyncWriteNow = side == 1 ? m_async_write1 : m_async_write2;
    auto &asyncWriteOppose = side == 1 ? m_async_write2 : m_async_write1;

    std::string type;
    getline(is, type);
    if (type == "exit") {
        end(side);
    } else if (type == "battle_ready") {
        if (side == 1) m_ready1 = true;
        else m_ready2 = true;
        if (m_ready1 && m_ready2) {
            makeProblem();
            m_async_write1(m_problemMsg);
            m_async_write2(m_problemMsg);
        }
    } else if (type == "poll_result") {
        auto &result = side == 1 ? m_result1 : m_result2;
        if (!result.empty()) {
            asyncWriteNow(result);
            result.clear();
            asyncWriteNow(m_problemMsg);
        } else {
            asyncWriteNow("no_battle_result\n");
        }
    } else if (type == "submit") {
        std::string answer;
        getline(is, answer);
        if (answer == m_problem.word()) {
            int expGainedNow = (1 + m_level) * 12;
            int expGainedOppose = (1 + m_level) * (-3);
            auto &resultNow = side == 1 ? m_result1 : m_result2;
            auto &resultOppose = side == 1 ? m_result2 : m_result1;
            resultNow = "battle_result\n1 "
                      + to_string(expGainedNow) + "\n"
                      + challengerNow.getInfo() + "\n";
            resultOppose = "battle_result\n3 "
                         + to_string(expGainedOppose) + "\n"
                         + challengerOppose.getInfo() + "\n";
            challengerNow.addExp(expGainedNow);
            challengerOppose.addExp(expGainedOppose);
            if (m_round == 10) {
                asyncWriteNow(resultNow);
            }
        } else {
            int expGainedNow = (1 + m_level) * (-6);
            int expGainedOppose = 0;
            auto &resultNow = side == 1 ? m_result1 : m_result2;
            auto &resultOppose = side == 1 ? m_result2 : m_result1;
            resultNow = "battle_result\n0 "
                      + to_string(expGainedNow) + "\n"
                      + challengerNow.getInfo() + "\n";
            resultOppose = "battle_result\n2 "
                         + to_string(expGainedOppose) + "\n"
                         + challengerOppose.getInfo() + "\n";
            challengerNow.addExp(expGainedNow);
            challengerOppose.addExp(expGainedOppose);
            if (m_round == 10) {
                asyncWriteNow(resultNow);
            }
        }
        // asio::steady_timer t(m_ioContext, asio::chrono::milliseconds(500));
        // auto self = shared_from_this();
        // t.async_wait([this, self](std::error_code ec) {
        //     sendProblem();
        // });
        if (m_round == 10) {
            m_ended = true;
            return false;
        }
        m_round++;
        makeProblem();
    }
    return !m_ended;
}

void Battle::end(int side) {
    if (!m_ended) {
        m_ended = true;
        if (side == 1) m_result2 = "battle_result\n4 0\n0 0 0 0\n";
        else m_result1 = "battle_result\n4 0\n0 0 0 0\n";
    }
}

void Battle::makeProblem() {
    m_problem = db.getRandomProblem(std::min(m_level, 6), m_level + 4);
    int totalRound = 10;
    int timeLimit = 30;
    m_problemMsg = "problem\n"
                 + m_problem.word() + "\n"
                 + to_string(m_level) + " "
                 + to_string(m_round) + " "
                 + to_string(totalRound) + " "
                 + to_string(timeLimit) + "\n";
}
