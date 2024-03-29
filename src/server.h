#ifndef __CPPNAT_SERVER_H__
#define __CPPNAT_SERVER_H__

#include "prefix.h"
#include "proxy_socket.h"
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
    std::uint16_t socket_id_;
    std::map<std::uint16_t, ProxySocketPtr> proxy_socket_map_;
    SocketPtr client_socket_ptr_;

    void Handshake(SocketPtr socket_ptr);
    void Proxy(SocketPtr socket_ptr);
    void AcceptClient(SocketPtr socket_ptr);
    void OnClientReadError(const std::error_code& ec);
    void OnClientWriteError(const std::error_code& ec);
    void InitMessageHandler();
    void BeginProxy(std::uint16_t id);
    void AcceptSocket();
};

NAMESPACE_CPPNAT_END

#endif