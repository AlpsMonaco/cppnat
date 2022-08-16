#ifndef __CPPNAT_SERVER_H__
#define __CPPNAT_SERVER_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

class Server
{
public:
    Server(const std::string& listen_addr, std::uint16_t port);
    ~Server();

    void Start();

protected:
    asio::io_service ios_;
    asio::ip::tcp::acceptor acceptor_;
    bool is_client_connected_;

    void Handshake(SocketPtr socket_ptr);
    void Proxy(SocketPtr socket_ptr);
    void AcceptClient(SocketPtr socket_ptr);
};

NAMESPACE_CPPNAT_END

#endif