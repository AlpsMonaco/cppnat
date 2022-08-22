#include "server.h"

#include "cs_message.h"
#include "handshake.h"
#include "log.h"

NAMESPACE_CPPNAT_START

Server::Server(const std::string& listen_addr, std::uint16_t port)
    : ios_(),
      acceptor_(ios_, asio::ip::tcp::endpoint(
                          asio::ip::address::from_string(listen_addr), port)),
      is_client_connected_(false),
      message_handler_(),
      message_writer_(),
      socket_id_(0),
      proxy_socket_map_(),
      client_socket_ptr_() {}

Server::~Server() { Stop(); }

void Server::InitMessageHandler()
{
    Log::Info("initialize message handler");
    message_handler_
        .Bind<ServerMessage::kNewProxyResult, ServerMessage::NewProxyResult>(
            [&](const ServerMessage::NewProxyResult& msg,
                MessageWriter& writer) -> void
            {
                if (msg.code == StatusCode::kSuccess)
                    BeginProxy(msg.id);
                else
                    proxy_socket_map_.erase(msg.id);
            });
    message_handler_
        .Bind<ServerMessage::kDataTransfer, ServerMessage::DataTransfer>(
            [&](const ServerMessage::DataTransfer& msg, MessageWriter& writer)
            {
                auto buffer_ptr = msg.Copy();
                proxy_socket_map_[msg.id]->Write(msg.Copy());
            });

    message_handler_.Bind<ServerMessage::kClientProxySocketClosed,
                          ServerMessage::ClientProxySocketClosed>(
        [&](const ServerMessage::ClientProxySocketClosed& msg,
            MessageWriter& writer)
        {
            proxy_socket_map_[msg.id]->Close();
            proxy_socket_map_.erase(msg.id);
        });
}

void Server::Stop()
{
    client_socket_ptr_->close();
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
    InitMessageHandler();
    AcceptSocket();
    Log::Info("wait for cppnat client");
    ios_.run();
}

void Server::AcceptSocket()
{
    SocketPtr socket_ptr = CreateSocket(ios_);
    acceptor_.async_accept(
        *socket_ptr, [socket_ptr, this](const asio::error_code& ec) -> void
        {
            if (ec)
            {
                HandleErrorCode(ec);
                return;
            }
            if (is_client_connected_)
                Proxy(socket_ptr);
            else
                Handshake(socket_ptr);
            AcceptSocket();
        });
}

void Server::Proxy(SocketPtr socket_ptr)
{
    Log::Info("ready to proxy {}:{}",
              socket_ptr->remote_endpoint().address().to_string(),
              socket_ptr->remote_endpoint().port());
    ClientMessage::NewProxy msg;
    msg.id = socket_id_++;
    Log::Info("new proxy id:{}", msg.id);
    proxy_socket_map_.emplace(msg.id,
                              std::make_shared<ProxySocket>(msg.id, socket_ptr));
    message_writer_.Write(ClientMessage::kNewProxy, msg);
}

void Server::OnClientWriteError(const std::error_code& ec)
{
    Log::Error("write to client error {} {}", ec.value(), ec.message());
}

void Server::BeginProxy(std::uint16_t id)
{
    Log::Info("id:{},begin proxy", id);
    auto proxy_socket_ptr = proxy_socket_map_[id];
    proxy_socket_ptr->SetOnRecv([proxy_socket_ptr,
                                 this](ProxyData& proxy_data) -> void
                                {
                                    auto ec = message_writer_.Write(ClientMessage::kDataTransfer, proxy_data);
                                    if (ec)
                                    {
                                        OnClientWriteError(ec);
                                        return;
                                    }
                                    proxy_socket_ptr->ReadOnce();
                                });
    proxy_socket_ptr->SetOnReadError(
        [proxy_socket_ptr, this](std::uint16_t id,
                                 const std::error_code& ec) -> void
        {
            HandleErrorCode(ec);
            if (is_client_connected_)
            {
                ClientMessage::ServerProxySocketClosed msg{id};
                auto err = message_writer_.Write(ClientMessage::kDataTransfer, msg);
                if (err) OnClientWriteError(ec);
            }
        });
    proxy_socket_ptr->SetOnWriteError(
        [proxy_socket_ptr, this](std::uint16_t id,
                                 const std::error_code& ec) -> void
        {
            HandleErrorCode(ec);
        });
    proxy_socket_ptr->ReadOnce();
}

void Server::Handshake(SocketPtr socket_ptr)
{
    Log::Info("handshaking with {}:{}",
              socket_ptr->remote_endpoint().address().to_string(),
              socket_ptr->remote_endpoint().port());
    TimerPtr timer_ptr = CreateTimer(ios_);
    timer_ptr->expires_from_now(std::chrono::seconds(10));
    timer_ptr->async_wait(
        [socket_ptr, timer_ptr, this](const std::error_code& ec) -> void
        {
            if (!ec) socket_ptr->close();
        });
    std::shared_ptr<BufferSize<64>> buffer_ptr =
        std::make_shared<BufferSize<64>>();
    asio::async_read(
        *socket_ptr,
        asio::buffer(buffer_ptr->Get(), Handshake::server_message.size()),
        [buffer_ptr, timer_ptr, socket_ptr, this](const std::error_code& ec,
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
                HandleErrorCode(ec);
                timer_ptr->cancel();
                return;
            }
            std::string_view recv_data(buffer_ptr->Get(), length);
            if (recv_data != Handshake::server_message)
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
                                      HandleErrorCode(ec);
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

void Server::OnClientReadError(const std::error_code& ec)
{
    Log::Error("disconnect from client {} {}", ec.value(), ec.message());
    is_client_connected_ = false;
    for (auto& v : proxy_socket_map_)
    {
        v.second->Close();
    }
    proxy_socket_map_.clear();
}

void Server::AcceptClient(SocketPtr socket_ptr)
{
    Log::Info("accept client from {}:{}",
              socket_ptr->remote_endpoint().address().to_string(),
              socket_ptr->remote_endpoint().port());
    auto session = std::make_shared<Session>(
        socket_ptr, message_handler_,
        [this](const std::error_code& ec) -> void
        {
            OnClientReadError(ec);
        });
    message_writer_ = session->GetMessageWriter();
    session->Start();
}

NAMESPACE_CPPNAT_END