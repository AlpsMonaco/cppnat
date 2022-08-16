#include "server.h"
#include "buffer.h"

NAMESPACE_CPPNAT_START

Server::Server(const std::string& listen_addr, std::uint16_t port)
    : ios_(),
      acceptor_(ios_, asio::ip::tcp::endpoint(
                          asio::ip::address::from_string(listen_addr), port)),
      is_client_connected_(false)
{
}

Server::~Server()
{
}

void Server::Start()
{
    SocketPtr socket_ptr = CreateSocket(ios_);
    acceptor_
        .async_accept(*socket_ptr,
                      [socket_ptr, this](const asio::error_code& ec) -> void
                      {
                          if (ec)
                          {
                              HandleError(ec);
                              return;
                          }
                          if (is_client_connected_)
                              Proxy(socket_ptr);
                          else
                              Handshake(socket_ptr);
                      });
}

void Server::Proxy(SocketPtr socket_ptr)
{
}

void Server::Handshake(SocketPtr socket_ptr)
{
    TimerPtr timer_ptr = CreateTimer(ios_);
    timer_ptr->expires_from_now(std::chrono::seconds(10));
    timer_ptr->async_wait(
        [socket_ptr, timer_ptr, this](const std::error_code& ec) -> void
        {
            if (!ec)
                if (!is_client_connected_)
                    socket_ptr->close();
        });
}

void Server::AcceptClient(SocketPtr socket_ptr)
{
}

NAMESPACE_CPPNAT_END