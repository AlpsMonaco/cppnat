#ifndef __CPPNAT_MESSAGE_H__
#define __CPPNAT_MESSAGE_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

template <typename T, typename = void>
struct IsMessageType {
  static constexpr bool value = false;
};

template <typename T>
struct IsMessageType<T, decltype(std::declval<T>().packet_size,
                                 std::declval<T>().cmd, void())> {
  static constexpr bool value = true;
};

template <typename T, typename = void>
struct MessageSize {
  static size_t Get(const T &t) { return sizeof(T); }
};

template <typename T>
struct MessageSize<T, decltype(std::declval<T>().Size(), void())> {
  static size_t Get(const T &t) { return const_cast<T &>(t).Size(); }
};

using Message = Protocol::Header;

class MessageWriter {
 public:
  MessageWriter(SocketPtr socket_ptr = nullptr,
                ErrorHandler error_handler = nullptr);
  ~MessageWriter();

  template <typename CmdType, typename T>
  std::error_code Write(CmdType cmd, T &t) {
    static_assert(IsMessageType<T>::value,
                  "message must inherit cppnat::Message");
    t.cmd = static_cast<Protocol::Cmd>(cmd);
    t.packet_size = MessageSize<T>::Get(t);
    std::error_code ec;
    asio::write(*socket_ptr_,
                asio::buffer(reinterpret_cast<const char *>(&t), t.packet_size),
                ec);
    return ec;
  }

 protected:
  SocketPtr socket_ptr_;
  ErrorHandler error_handler_;
};

class MessageHandler {
 public:
  MessageHandler();
  ~MessageHandler();

  template <Protocol::Cmd cmd, typename T>
  void Bind(const std::function<void(const T &, MessageWriter &)> &callback) {
    callback_map_[cmd] = [callback](const char *raw_data,
                                    MessageWriter &writer) -> void {
      callback(*reinterpret_cast<const T *>(raw_data), writer);
    };
  }

  void Handle(Protocol::Cmd, const char *, SocketPtr socket_ptr);

  void Handle(Protocol::Cmd, const char *, SocketPtr socket_ptr) const;

 protected:
  std::map<Protocol::Cmd, std::function<void(const char *, MessageWriter &)> >
      callback_map_;
};

NAMESPACE_CPPNAT_END

#endif