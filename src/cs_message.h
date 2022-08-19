#ifndef __CPPNAT_CS_MESSAGE_H__
#define __CPPNAT_CS_MESSAGE_H__

#include "message.h"

NAMESPACE_CPPNAT_START

enum class StatusCode : std::uint16_t { kSuccess, kError };

class ServerMessage {
 public:
  enum MessageEnum {
    kNewProxyResult = 10001,
    kClientProxySocketClosed,
    kDataTransfer,
  };

  struct NewProxyResult : public Message {
    std::uint16_t id;
    StatusCode code;
  };

  struct ClientProxySocketClosed : public Message {
    std::uint16_t id;

    ClientProxySocketClosed(size_t id) : id(id) {}
  };

  struct DataTransfer : public Message {
    std::uint16_t id;
    std::uint16_t data_size;
    static constexpr size_t body_meta_size = sizeof(id) + sizeof(data_size);
    static constexpr size_t writable_size =
        Protocol::max_size - Protocol::header_size - body_meta_size;
    char data[writable_size];

    size_t Size() { return body_meta_size + data_size; }

    DynamicBufferPtr Copy() {
      auto buffer_ptr = std::make_shared<DynamicBuffer>(data_size);
      buffer_ptr->Write(data, data_size);
      return buffer_ptr;
    }

    DynamicBufferPtr Copy() const {
      return const_cast<DataTransfer &>(*this).Copy();
    }
  };
};

class ClientMessage {
 public:
  enum MessageEnum {
    kNewProxy = 10001,
    kServerProxySocketClosed,
    kDataTransfer,
  };

  struct NewProxy : public Message {
    size_t id;
  };

  using NewProxyResult = ServerMessage::NewProxyResult;
  using ServerProxySocketClosed = ServerMessage::ClientProxySocketClosed;
  using DataTransfer = ServerMessage::DataTransfer;
};

using ProxyData = ClientMessage::DataTransfer;

NAMESPACE_CPPNAT_END

#endif