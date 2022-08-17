#ifndef __CPPNAT_CS_MESSAGE_H__
#define __CPPNAT_CS_MESSAGE_H__

#include "message.h"

NAMESPACE_CPPNAT_START

enum class MessageEnum : Protocol::Cmd
{
    kNewProxy = 1000,
    kNewProxyResult,
    kSocketClosed,
    kDataTransfer
};

enum class StatusCode : std::uint16_t
{
    kSuccess,
    kError
};

class ServerMessage
{
public:
    struct NewProxy : public Message
    {
        size_t id;
    };

    struct NewProxyResult : public Message
    {
        size_t id;
        StatusCode code;
    };

    struct ServerProxySocketClosed : public Message
    {
        size_t id;
    };

    struct DataTransfer : public Message
    {
        size_t id;
        size_t data_size;
        static constexpr size_t body_meta_size = sizeof(id) + sizeof(data_size);
        static constexpr size_t writable_size = Protocol::max_size - Protocol::header_size - body_meta_size;
        char data[writable_size];

        size_t Size()
        {
            return body_meta_size + data_size;
        }
    };
};

class ClientMessage
{
public:
    using NewProxy = ServerMessage::NewProxy;
    using NewProxyResult = ServerMessage::NewProxyResult;
    using ClientProxySocketClosed = ServerMessage::ServerProxySocketClosed;
    using DataTransfer = ServerMessage::DataTransfer;
};

using ProxyData = ClientMessage::DataTransfer;

NAMESPACE_CPPNAT_END

#endif