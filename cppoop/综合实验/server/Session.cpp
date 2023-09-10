#include "Session.h"
#include "Database.h"
#include <iostream>

using std::string, std::getline, std::cout, std::to_string;

std::unordered_set<std::string> logged;

std::shared_ptr<Session> Session::s_matching;

std::string _;

Session::Session(asio::io_context &ioContext, tcp::socket &&socket)
    : m_ioContext(ioContext), m_socket(std::move(socket)), m_state(SessionState::init) {
    std::cout << "Session" << std::endl;
}

Session::~Session() {
    if (m_battle != nullptr) {
        m_battle->end(m_side);
    }
    if (m_state == SessionState::matching) {
        s_matching.reset();
    }
    if (m_user) {
       logged.erase(m_user->getName());
    }
    std::cout << "~Session" << std::endl;
}

void Session::start() {
    async_read();
}

void Session::sendProblem() {
    m_problem = db.getRandomProblem(std::min(m_level, 6), m_level + 4);
    int totalRound = getTotalRound();
    int timeLimit = getTimeLimit();

    async_write("problem\n"
                + m_problem.word() + "\n"
                + to_string(m_level) + " "
                + to_string(m_round) + " "
                + to_string(totalRound) + " "
                + to_string(timeLimit) + "\n");
    if (m_round == 1) {
        m_levelStartTime = std::chrono::steady_clock::now();
    }
}

int Session::getTotalRound() {
    if (m_level < 2) return 1;
    if (m_level < 4) return 2;
    if (m_level < 6) return 3;
    if (m_level < 8) return 4;
    return 5;
}

int Session::getTimeLimit() {
    if (m_level < 5) return 40;
    else if (m_level < 10) return 30;
    else if (m_level < 15) return 25;
    else if (m_level < 20) return 20;
    return 15;
}

void Session::handle() {
    std::cout << m_msg << "\n";
    if (m_state == SessionState::init) handle_init();
    else if (m_state == SessionState::challengerLogined) handle_challengerLogined();
    else if (m_state == SessionState::authorLogined) handle_authorLogined();
    else if (m_state == SessionState::inGame) handle_inGame();
    else if (m_state == SessionState::waitForRetry) handle_waitForRetry();
    else if (m_state == SessionState::matching) handle_matching();
    else if (m_state == SessionState::battle) {
        if (!m_battle->handle(m_side, m_msg)) {
            m_state = SessionState::challengerLogined;
            m_battle.reset();
        }
    }
    if (db.unsaved()) {
        db.save();
    }
}

void Session::handle_init() {
    std::istringstream is(m_msg);
    string type;
    getline(is, type);
    if (type == "signup") {
        std::string name, password;
        int userTypeId;
        is >> userTypeId;
        std::getline(is, _);
        UserType userType = static_cast<UserType>(userTypeId);
        getline(is, name);
        getline(is, password);
        UserPtr user;
        if (userType == UserType::challenger) {
            user = std::make_shared<Challenger>(name, password);
        } else if (userType == UserType::author) {
            user = std::make_shared<Author>(name, password);
        }
        std::string response;
        if (user == nullptr) {
            response = "用户类型无效\n";
        } else if (db.addUser(user)) {
            response = "success\n";
        } else {
            response = "用户名已存在\n ";
        }
        async_write("signup_res\n" + response);

    } else if (type == "login") {
        string name, password;
        getline(is, name);
        getline(is, password);
        std::string response;

        auto result = db.getUserByName(name);
        if (result == nullptr) {
            response = "没有此用户\n";
        } else if (result->getPassword() == password) {
            if (logged.count(name)) {
                response = "已经登陆过了\n";
            } else {
                logged.insert(name);
                response = "success\n";
                m_user = result;
                auto userType = result->getType();
                response += to_string(static_cast<int>(userType)) + "\n";
                if (userType == UserType::challenger) {
                    auto challenger = std::static_pointer_cast<Challenger>(result);
                    response += challenger->getInfo();
                    m_state = SessionState::challengerLogined;
                } else if (userType == UserType::author) {
                    auto author = std::static_pointer_cast<Author>(result);
                    response += author->getInfo();
                    m_state = SessionState::authorLogined;
                }
            }
        } else {
            response = "密码错误\n";
        }
        async_write("login_res\n" + response);
    }
}

void Session::handle_challengerLogined() {
    auto challenger = std::static_pointer_cast<Challenger>(m_user);
    std::istringstream is(m_msg);
    string type;
    getline(is, type);
    if (type == "logout") {
        logged.erase(m_user->getName());
        m_state = SessionState::init;
    } else if (type == "play") {
        m_level = 1;
        m_round = 1;
        m_retry = 2;
        sendProblem();
        m_state = SessionState::inGame;
    } else if (type == "start_match") {
        if (s_matching == nullptr) {
            m_side = 1;
            s_matching = shared_from_this();
        } else {
            m_side = 2;
            m_battle = std::make_shared<Battle>(
                *std::static_pointer_cast<Challenger>(s_matching->m_user),
                *std::static_pointer_cast<Challenger>(m_user),
                std::bind(&Session::async_write, s_matching, std::placeholders::_1),
                std::bind(&Session::async_write, this, std::placeholders::_1));
            s_matching->m_battle = m_battle;
            s_matching = nullptr;
        }
        m_state = SessionState::matching;
    } else if (type == "userlist") {
        async_write(db.getUserListForClient());
    }
}

void Session::handle_authorLogined() {
    auto author = std::static_pointer_cast<Author>(m_user);
    std::istringstream is(m_msg);
    string type;
    getline(is, type);
    if (type == "logout") {
        logged.erase(m_user->getName());
        m_state = SessionState::init;
    } else if (type == "make_problem") {
        string word;
        getline(is, word);
        std::string response;
        if (db.addProblem(Problem(word))) {
            response = "success\n";
            author->addProblem();
        } else {
            response = "该单词已添加\n";
        }
        async_write("make_problem_res\n" + response + author->getInfo());
    } else if (type == "userlist") {
        async_write(db.getUserListForClient());
    }
}

void Session::handle_inGame() {
    auto challenger = std::static_pointer_cast<Challenger>(m_user);
    std::istringstream is(m_msg);
    string type;
    getline(is, type);
    if (type == "exit") {
        m_state = SessionState::challengerLogined;
    } else if (type == "submit") {
        std::string answer;
        getline(is, answer);
        if (answer == m_problem.word()) {
            int duration = 0, expGained = 0;
            if (m_round < getTotalRound()) {
                m_round++;
            } else {
                auto now = std::chrono::steady_clock::now();
                duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_levelStartTime).count() / 100;
                int totalRound = getTotalRound();
                int timeLimit = getTimeLimit();
                double secondPerRound = ((duration - 5 * (totalRound - 1)) / totalRound - timeLimit) / 10.0;
                expGained = (1 + m_level) * (4 + 8 / (secondPerRound + 1));
                m_round = 1;
                m_level++;
                challenger->passLevel();
                challenger->addExp(expGained);
            }
            async_write("result\n1\n"
                        + to_string(duration) + " " + to_string(expGained) + " " + to_string(m_retry) + "\n"
                        + challenger->getInfo());
            asio::steady_timer t(m_ioContext, asio::chrono::milliseconds(500));
            auto self = shared_from_this();
            t.async_wait([this, self](std::error_code ec) {
                sendProblem();
            });
        } else {
            async_write("result\n0\n0 0 " + to_string(m_retry) + "\n"
                        + challenger->getInfo());
            m_state = SessionState::waitForRetry;
        }
    }
}

void Session::handle_waitForRetry() {
    auto challenger = std::static_pointer_cast<Challenger>(m_user);
    std::istringstream is(m_msg);
    string type;
    getline(is, type);
    if (type == "retry") {
        if (m_retry > 0) {
            m_retry--;
            m_round = 1;
            sendProblem();
            m_state = SessionState::inGame;
        }
    } else if (type == "exit") {
        m_state = SessionState::challengerLogined;
    }
}

void Session::handle_matching() {
    auto challenger = std::static_pointer_cast<Challenger>(m_user);
    std::istringstream is(m_msg);
    string type;
    getline(is, type);
    if (type == "stop_match") {
        s_matching = nullptr;
        m_state = SessionState::challengerLogined;
    } else if (type == "poll_match") {
        if (m_battle) {
            async_write("match_res\n1\n");
            m_state = SessionState::battle;
        } else {
            async_write("match_res\n0\n");
        }
    }
}

void Session::async_read() {
    auto self(shared_from_this());
    asio::async_read_until(m_socket, m_inbuf, '\0',
                           [this, self](std::error_code ec, std::size_t length) {
                               if (ec) {
                                   std::cout << "connection closed: " << ec.message() << std::endl;
                                   m_socket.close();
                               } else {
                                   std::istream inbufStream(&m_inbuf);
                                   std::getline(inbufStream, m_msg, '\0');
                                   handle();
                                   async_read();
                               }
                           });
}

void Session::async_write(const std::string &s) {
    auto self(shared_from_this());
    asio::async_write(m_socket, asio::buffer(s.c_str(), s.length() + 1),
                      [this, self](std::error_code ec, std::size_t length) {
                          if (ec) {
                              std::cout << "connection closed: " << ec.message() << std::endl;
                              m_socket.close();
                          }
                      });
}