#ifndef __CPPNAT_MESSAGE_H__
#define __CPPNAT_MESSAGE_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

template <typename T, typename = void>
struct IsMessageType
{
    static constexpr bool value = false;
};

template <typename T>
struct IsMessageType<T,
                     decltype(std::declval<T>().packet_size,
                              std::declval<T>().cmd,
                              void())>
{
    static constexpr bool value = true;
};

template <typename T, typename = void>
struct MessageSize
{
    static size_t Get(T& t)
    {
        return sizeof(T);
    }
};

template <typename T>
struct MessageSize<T, decltype(std::declval<T>().Size(), void())>
{
    static size_t Get(T& t)
    {
        return t.Size();
    }
};

using Message = Protocol::Header;
class MessageWriter
{
public:
    MessageWriter(SocketPtr socket_ptr = nullptr,
                  ErrorHandlerPtr error_handler_ptr = nullptr);
    ~MessageWriter();

    template <typename CmdType, typename T>
    void Write(CmdType cmd, T& t)
    {
        static_assert<IsMessageType<T>::value, "message must inherit cppnat::Message">;
        t.cmd = static_cast<Protocol::Cmd>(cmd);
        t.packet_size = MessageSize<T>::Get(t);
        asio::async_write(asio::buffer(
                              reinterpret_cast<const char*>(&t), MessageSize<T>::Get(t)),
                          [](const std::error_code& ec) -> void
                          {
                              if (ec)
                                  (*error_handler_ptr_)(ec);
                          })
    }

protected:
    SocketPtr socket_ptr_;
    ErrorHandlerPtr error_handler_ptr_;
};

class MessageHandler
{
public:
    MessageHandler();
    ~MessageHandler();

    template <typename T>
    void Bind(Protocol::Cmd, const std::function<void(const T&, MessageWriter&)>&);
    void Handle(Protocol::Cmd, const char*, SocketPtr socket_ptr);

    void Handle(Protocol::Cmd, const char*, SocketPtr socket_ptr) const;

protected:
    std::map<Protocol::Cmd, std::function<void(const char*, MessageWriter&)>> callback_map_;
};

NAMESPACE_CPPNAT_END

#endif