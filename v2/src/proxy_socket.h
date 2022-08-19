#ifndef __CPP_NAT_PROXY_SOCKET_H__
#define __CPP_NAT_PROXY_SOCKET_H__

#include "cs_message.h"
#include "prefix.h"

NAMESPACE_CPPNAT_START

class ProxySocket : public std::enable_shared_from_this<ProxySocket> {
 public:
  using OnRecvCallback = std::function<void(ProxyData &)>;
  using OnReadErrorCallback =
      std::function<void(std::uint16_t, const std::error_code &)>;
  using OnWriteErrorCallback =
      std::function<void(std::uint16_t, const std::error_code &)>;

  ProxySocket(std::uint16_t id, SocketPtr socket_ptr);
  ~ProxySocket();

  void ReadOnce();
  void Write(DynamicBufferPtr);
  void Close();

  void SetOnRecv(const OnRecvCallback &);
  void SetOnReadError(const OnReadErrorCallback &);
  void SetOnWriteError(const OnWriteErrorCallback &);

 protected:
  SocketPtr socket_ptr_;
  ProxyData proxy_data_;
  OnRecvCallback on_recv_;
  OnReadErrorCallback on_read_error_;
  OnWriteErrorCallback on_write_error_;
};

using ProxySocketPtr = std::shared_ptr<ProxySocket>;

NAMESPACE_CPPNAT_END

#endif