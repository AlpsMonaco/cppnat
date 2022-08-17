#include "server.h"
#include "handshake.h"
#include "cs_message.h"

NAMESPACE_CPPNAT_START

Server::Server(const std::string& listen_addr, std::uint16_t port)
    : ios_(),
      acceptor_(ios_,
                asio::ip::tcp::endpoint(asio::ip::address::from_string(listen_addr), port)),
      is_client_connected_(false),
      message_handler_(),
      message_writer_(),
      socket_id_(0)
{
}

Server::~Server()
{
    Stop();
}

void Server::Stop()
{
    ios_.stop();
    int count = 0;
    for (;;)
    {
        count++;
        if (count == 60) return;
        if (ios_.stopped()) return;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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
    ServerMessage::NewProxy msg;
    msg.id = socket_id_++;
    message_writer_.Write(MessageEnum::kNewProxy, msg);
}

void Server::Handshake(SocketPtr socket_ptr)
{
    TimerPtr timer_ptr = CreateTimer(ios_);
    timer_ptr->expires_from_now(std::chrono::seconds(10));
    timer_ptr->async_wait(
        [socket_ptr, timer_ptr, this](const std::error_code& ec) -> void
        {
            if (!ec)
                socket_ptr->close();
        });
    std::shared_ptr<BufferSize<64>> buffer_ptr = std::make_shared<BufferSize<64>>();
    asio::async_read(*socket_ptr,
                     asio::buffer(buffer_ptr->Get(), Handshake::server_message.size()),
                     [buffer_ptr, timer_ptr,
                      socket_ptr, this](
                         const std::error_code& ec,
                         size_t length) -> void
                     {
                         if (is_client_connected_)
                         {
                             timer_ptr->cancel();
                             socket_ptr->close();
                             return;
                         }
                         if (ec)
                         {
                             HandleError(ec);
                             timer_ptr->cancel();
                             return;
                         }
                         if (
                             std::string_view(buffer_ptr->Get(),
                                              Handshake::server_message.size()) !=
                             Handshake::server_message)
                         {
                             timer_ptr->cancel();
                             socket_ptr->close();
                             return;
                         }
                         asio::async_write(*socket_ptr,
                                           asio::buffer(Handshake::client_message.data(),
                                                        Handshake::client_message.size()),
                                           [buffer_ptr, timer_ptr, socket_ptr, this](
                                               const std::error_code& ec, size_t) -> void
                                           {
                                               if (is_client_connected_)
                                               {
                                                   timer_ptr->cancel();
                                                   socket_ptr->close();
                                                   return;
                                               }
                                               if (ec)
                                               {
                                                   HandleError(ec);
                                                   timer_ptr->cancel();
                                                   socket_ptr->close();
                                                   return;
                                               }
                                               AcceptClient(socket_ptr);
                                               is_client_connected_ = true;
                                               timer_ptr->cancel();
                                           });
                     });
}

void Server::OnClientSocketError(const std::error_code& ec)
{
}

void Server::AcceptClient(SocketPtr socket_ptr)
{
    auto session = std::make_shared<Session>(socket_ptr,
                                             message_handler_,
                                             [this](const std::error_code& ec) -> void
                                             {
                                                 OnClientSocketError(ec);
                                             });
    message_writer_ = session->GetMessageWriter();
    session->Start();
}

NAMESPACE_CPPNAT_END