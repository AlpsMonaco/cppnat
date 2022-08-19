#ifndef __CPPNAT_CLIENT_H__
#define __CPPNAT_CLIENT_H__

#include "cs_message.h"
#include "prefix.h"
#include "session.h"
#include "proxy_socket.h"

NAMESPACE_CPPNAT_START

class Client {
 public:
  Client(const std::string &server_addr, std::uint16_t server_port,
         const std::string &proxy_addr, std::uint16_t proxy_port);
  ~Client();

  void Start();
  void Stop();

 protected:
  asio::io_service ios_;
  asio::ip::tcp::endpoint server_endpoint_;
  asio::ip::tcp::endpoint proxy_endpoint_;
  SocketPtr server_socket_ptr_;
  MessageHandler message_handler_;
  MessageWriter message_writer_;
  std::map<size_t, ProxySocketPtr> proxy_socket_map_;

  void Handshake();
  void BeginMessageLoop();
  void InitMessageHandler();
  void OnServerSocketClosed(const std::error_code &ec);
  void BeginProxy(std::uint16_t id);
};

NAMESPACE_CPPNAT_END

#endif