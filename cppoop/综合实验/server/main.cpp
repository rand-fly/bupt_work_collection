#include "Database.h"
#include "Session.h"
#include "asio.hpp"
#include <User.h>
#include <iostream>
#include <memory>

using asio::ip::tcp;
const short port = 1764; // yh's number

class Server {
  public:
    Server(asio::io_context &io_context, short port)
        : m_ioContext(io_context), m_acceptor(io_context, tcp::endpoint(tcp::v4(), port), false) {
        do_accept();
    }

  private:
    void do_accept() {
        m_acceptor.async_accept([this](std::error_code ec, tcp::socket socket) {
            if (ec) {
                std::cout << "async_accept error: " << ec.message() << std::endl;
            } else {
                std::make_shared<Session>(m_ioContext, std::move(socket))->start();
            }
            do_accept();
        });
    }

    asio::io_context &m_ioContext;
    tcp::acceptor m_acceptor;
};

int main(int argc, char *argv[]) {
    db.load();
    try {
        asio::io_context io_context;
        Server s(io_context, port);
        std::cout << "listening on port " << port << std::endl;
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}