#include "Socket.h"
#include <iostream>

using asio::ip::tcp;

Socket::~Socket() {
    disconnect();
}

bool Socket::connect(std::string servername) {
    tcp::resolver resolver(m_ioContext);
    size_t pos = servername.find(':');
    std::string host = servername.substr(0, pos);
    std::string port = servername.substr(pos + 1);
    try {
        asio::connect(m_socket, resolver.resolve(host, port));
        return true;
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
        m_socket.close();
        return false;
    }
}

void Socket::disconnect() {
    m_socket.close();
}

void Socket::write(std::string s) {
    try {
        asio::write(m_socket, asio::buffer(s.c_str(), s.length() + 1));
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
        m_socket.close();
    }
}

std::string Socket::read() {
    try {
        size_t len = asio::read_until(m_socket, m_inbuf, '\0');
        std::istream is(&m_inbuf);
        std::string s;
        std::getline(is, s, '\0');
        return s;
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
        m_socket.close();
        return "";
    }
}

std::istringstream Socket::readStream() {
    return std::istringstream(read());
}
