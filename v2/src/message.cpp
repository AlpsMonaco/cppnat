#include "message.h"

NAMESPACE_CPPNAT_START

MessageWriter::MessageWriter(SocketPtr socket_ptr, ErrorHandlerPtr error_handler_ptr)
    : socket_ptr_(socket_ptr),
      error_handler_ptr_(error_handler_ptr)
{
}

MessageWriter::~MessageWriter()
{
}

MessageHandler::MessageHandler()
    : callback_map_()
{
}

MessageHandler::~MessageHandler()
{
}

template <typename T>
void MessageHandler::Bind(Protocol::Cmd cmd,
                          const std::function<void(const T& t, MessageWriter& writer)>& callback)
{
    callback_map_[cmd] = [callback](const char* raw_data,
                                    MessageWriter& writer) -> void
    {
        callback(*reinterpret_cast<const T*>(raw_data), writer)
    };
}

void MessageHandler::Handle(Protocol::Cmd cmd, const char* data, SocketPtr socket_ptr)
{
    MessageWriter writer(socket_ptr);
    callback_map_[cmd](data, writer);
}

void MessageHandler::Handle(Protocol::Cmd cmd, const char* data, SocketPtr socket_ptr) const
{
    const_cast<MessageHandler&>(*this).Handle(cmd, data, socket_ptr);
}

NAMESPACE_CPPNAT_END