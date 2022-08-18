#include "client.h"

#include "handshake.h"

NAMESPACE_CPPNAT_START

Client::Client(const std::string &server_addr, std::uint16_t server_port,
               const std::string &proxy_addr, std::uint16_t proxy_port)
    : ios_(),
      server_endpoint_(asio::ip::address::from_string(server_addr),
                       server_port),
      proxy_endpoint_(asio::ip::address::from_string(proxy_addr), proxy_port),
      server_socket_ptr_(),
      message_handler_(),
      message_writer_(),
      proxy_socket_map_() {}

Client::~Client() {}

void Client::Stop() {
  server_socket_ptr_->close();
  ios_.stop();
  int count = 0;
  for (;;) {
    count++;
    if (count == 60) return;
    if (ios_.stopped()) return;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void Client::Start() {
  InitMessageHandler();
  server_socket_ptr_ = std::make_shared<asio::ip::tcp::socket>(ios_);
  server_socket_ptr_->async_connect(server_endpoint_,
                                    [&](const std::error_code &ec) -> void {
                                      if (ec) {
                                        HandleError(ec);
                                        return;
                                      }
                                      Handshake();
                                    });
  ios_.run();
}

void Client::BeginMessageLoop() {
  auto session = std::make_shared<Session>(
      server_socket_ptr_, message_handler_,
      [&](const std::error_code &ec) -> void { OnServerSocketClosed(ec); });
  message_writer_ = session->GetMessageWriter();
  session->Start();
}

void Client::OnServerSocketClosed(const std::error_code &ec) {
  HandleError(ec);
  for (auto &it : proxy_socket_map_) {
    it.second->Close();
    proxy_socket_map_.erase(it.first);
  }
}

void Client::InitMessageHandler() {
  message_handler_.Bind<ClientMessage::kNewProxy, ClientMessage::NewProxy>(
      [&](const ClientMessage::NewProxy &msg, MessageWriter &writer) -> void {
        auto socket_ptr = std::make_shared<asio::ip::tcp::socket>(ios_);
        size_t id = msg.id;
        socket_ptr->async_connect(
            proxy_endpoint_,
            [this, socket_ptr, id](const std::error_code &ec) -> void {
              ServerMessage::NewProxyResult result;
              result.id = id;
              if (ec) {
                HandleError(ec);
                result.code = StatusCode::kError;
              } else {
                proxy_socket_map_[id] =
                    std::make_shared<ProxySocket>(id, socket_ptr);
                result.code = StatusCode::kSuccess;
              }
              auto err =
                  message_writer_.Write(ServerMessage::kNewProxyResult, result);
              if (err) OnServerSocketClosed(err);
            });
      });
  message_handler_
      .Bind<ClientMessage::kDataTransfer, ClientMessage::DataTransfer>(
          [&](const ClientMessage::DataTransfer &msg, MessageWriter &writer) {
            auto buffer_ptr = msg.Copy();
            proxy_socket_map_[msg.id]->Write(msg.Copy());
          });

  message_handler_.Bind<ClientMessage::kServerProxySocketClosed,
                        ClientMessage::ServerProxySocketClosed>(
      [&](const ServerMessage::ClientProxySocketClosed &msg,
          MessageWriter &writer) {
        proxy_socket_map_[msg.id]->Close();
        proxy_socket_map_.erase(msg.id);
      });
}

void Client::Handshake() {
  asio::async_write(
      *server_socket_ptr_,
      asio::buffer(Handshake::server_message.data(),
                   Handshake::server_message.size()),
      [&](const std::error_code &ec, size_t) -> void {
        if (ec) {
          HandleError(ec);
          return;
        }
        std::shared_ptr<BufferSize<64>> buffer_ptr =
            std::make_shared<BufferSize<64>>();
        asio::async_read(
            *server_socket_ptr_,
            asio::buffer(buffer_ptr->Get(), Handshake::client_message.size()),
            [&, buffer_ptr](const std::error_code &ec, size_t length) -> void {
              if (ec) {
                HandleError(ec);
                return;
              }
              if (std::string_view(buffer_ptr->Get(), length) ==
                  Handshake::client_message) {
                BeginMessageLoop();
              }
            });
      });
}

NAMESPACE_CPPNAT_END