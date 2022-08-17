#ifndef __CPPNAT_SERVER_H__
#define __CPPNAT_SERVER_H__

#include "prefix.h"
#include "session.h"

NAMESPACE_CPPNAT_START

class Server
{
public:
    Server(const std::string& listen_addr, std::uint16_t port);
    ~Server();

    void Start();
    void Stop();

protected:
    asio::io_service ios_;
    asio::ip::tcp::acceptor acceptor_;
    bool is_client_connected_;
    MessageHandler message_handler_;
    MessageWriter message_writer_;
    size_t socket_id_;

    void Handshake(SocketPtr socket_ptr);
    void Proxy(SocketPtr socket_ptr);
    void AcceptClient(SocketPtr socket_ptr);
    void OnClientSocketError(const std::error_code&ec);
};

NAMESPACE_CPPNAT_END

#endif