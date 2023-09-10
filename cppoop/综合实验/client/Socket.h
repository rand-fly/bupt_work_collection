#pragma once

#include "asio.hpp"
#include <iostream>

class Socket {
  public:
    ~Socket();

    bool connect(std::string servername);

    void disconnect();

    void write(std::string s);

    std::string read();

    std::istringstream readStream();

  private:
    asio::io_context m_ioContext;
    asio::ip::tcp::socket m_socket{m_ioContext};
    asio::streambuf m_inbuf;
};