#pragma once
#include "Problem.h"
#include "User.h"
#include "asio.hpp"
#include <memory>
#include "Battle.h"

using asio::ip::tcp;

enum class SessionState {
    init,
    challengerLogined,
    authorLogined,
    inGame,
    waitForRetry,
    matching,
    battle
};

class Session : public std::enable_shared_from_this<Session> {
  public:
    Session::Session(asio::io_context &ioContext, tcp::socket &&socket);
    ~Session();
    void start();

  private:
    void sendProblem();
    int getTotalRound();
    int getTimeLimit();

    void handle();
    void handle_init();
    void handle_challengerLogined();
    void handle_authorLogined();
    void handle_inGame();
    void handle_waitForRetry();
    void handle_matching();

    void async_read();
    void async_write(const std::string &s);

    asio::io_context &m_ioContext;
    tcp::socket m_socket;
    asio::streambuf m_inbuf, m_outbuf;
    std::string m_msg;
    SessionState m_state;
    UserPtr m_user;

    int m_level, m_round, m_retry;
    std::chrono::steady_clock::time_point m_levelStartTime;
    Problem m_problem{""};

    static std::shared_ptr<Session> s_matching;
    std::shared_ptr<Battle> m_battle;
    int m_side;
};