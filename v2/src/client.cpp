#include "client.h"

#include "handshake.h"
#include "log.h"

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
  Log::Info("server {}:{},proxy to {}:{}",
            server_endpoint_.address().to_string(), server_endpoint_.port(),
            proxy_endpoint_.address().to_string(), proxy_endpoint_.port());
  InitMessageHandler();
  server_socket_ptr_ = std::make_shared<asio::ip::tcp::socket>(ios_);
  Log::Info("connection to server {}:{}",
            server_endpoint_.address().to_string(), server_endpoint_.port());
  server_socket_ptr_->async_connect(server_endpoint_,
                                    [&](const std::error_code &ec) -> void {
                                      if (ec) {
                                        HandleErrorCode(ec);
                                        return;
                                      }
                                      Handshake();
                                    });
  ios_.run();
}

void Client::BeginMessageLoop() {
  Log::Info("begin message loop");
  auto session = std::make_shared<Session>(
      server_socket_ptr_, message_handler_,
      [&](const std::error_code &ec) -> void { OnServerReadError(ec); });
  message_writer_ = session->GetMessageWriter();
  session->Start();
}

void Client::OnServerReadError(const std::error_code &ec) {
  Log::Error("disconnect from server {} {}", ec.value(), ec.message());
  for (auto &v : proxy_socket_map_) {
    v.second->Close();
  }
  proxy_socket_map_.clear();
}

void Client::OnServerWriteError(const std::error_code &ec) {
  Log::Error("write to server error {} {}", ec.value(), ec.message());
}

void Client::BeginProxy(std::uint16_t id) {
  auto proxy_socket_ptr = proxy_socket_map_[id];
  proxy_socket_ptr->SetOnRecv([proxy_socket_ptr,
                               this](ProxyData &proxy_data) -> void {
    auto ec = message_writer_.Write(ServerMessage::kDataTransfer, proxy_data);
    if (ec) {
      OnServerWriteError(ec);
      return;
    }
    proxy_socket_ptr->ReadOnce();
  });
  proxy_socket_ptr->SetOnReadError(
      [proxy_socket_ptr, this](std::uint16_t id,
                               const std::error_code &ec) -> void {
        HandleErrorCode(ec);
        ServerMessage::ClientProxySocketClosed msg{id};
        auto err =
            message_writer_.Write(ServerMessage::kClientProxySocketClosed, msg);
        if (err) {
          OnServerWriteError(err);
        }
      });
  proxy_socket_ptr->SetOnWriteError(
      [proxy_socket_ptr, this](std::uint16_t id,
                               const std::error_code &ec) -> void {
        HandleErrorCode(ec);
      });
  proxy_socket_ptr->ReadOnce();
}

void Client::InitMessageHandler() {
  Log::Info("initialize message handler");
  message_handler_.Bind<ClientMessage::kNewProxy, ClientMessage::NewProxy>(
      [&](const ClientMessage::NewProxy &msg, MessageWriter &writer) -> void {
        auto socket_ptr = std::make_shared<asio::ip::tcp::socket>(ios_);
        std::uint16_t id = msg.id;
        Log::Info("new proxy request,id:{}", id);
        socket_ptr->async_connect(
            proxy_endpoint_,
            [this, socket_ptr, id](const std::error_code &ec) -> void {
              ServerMessage::NewProxyResult result;
              result.id = id;
              if (ec) {
                HandleErrorCode(ec);
                result.code = StatusCode::kError;
              } else {
                proxy_socket_map_[id] =
                    std::make_shared<ProxySocket>(id, socket_ptr);
                result.code = StatusCode::kSuccess;
                BeginProxy(id);
              }
              auto err =
                  message_writer_.Write(ServerMessage::kNewProxyResult, result);
              if (err) OnServerWriteError(err);
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
  Log::Info("handshaking with server");
  asio::async_write(
      *server_socket_ptr_,
      asio::buffer(Handshake::server_message.data(),
                   Handshake::server_message.size()),
      [&](const std::error_code &ec, size_t) -> void {
        if (ec) {
          HandleErrorCode(ec);
          return;
        }
        std::shared_ptr<BufferSize<64>> buffer_ptr =
            std::make_shared<BufferSize<64>>();
        asio::async_read(
            *server_socket_ptr_,
            asio::buffer(buffer_ptr->Get(), Handshake::client_message.size()),
            [&, buffer_ptr](const std::error_code &ec, size_t length) -> void {
              if (ec) {
                HandleErrorCode(ec);
                return;
              }
              std::string_view recv_data(buffer_ptr->Get(), length);
              Log::Bytes(recv_data, "handshake recv");
              if (recv_data == Handshake::client_message) {
                BeginMessageLoop();
                Log::Info("handshake successfully");
              }
            });
      });
}

NAMESPACE_CPPNAT_END