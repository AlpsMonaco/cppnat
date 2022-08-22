#include "message.h"

NAMESPACE_CPPNAT_START

MessageWriter::MessageWriter(SocketPtr socket_ptr, ErrorHandler error_handler)
    : socket_ptr_(socket_ptr), error_handler_(error_handler) {}
MessageWriter::~MessageWriter() {}

MessageHandler::MessageHandler() : callback_map_() {}
MessageHandler::~MessageHandler() {}

void MessageHandler::Handle(Protocol::Cmd cmd, const char* data,
                            SocketPtr socket_ptr)
{
    MessageWriter writer(socket_ptr);
    callback_map_[cmd](data, writer);
}

void MessageHandler::Handle(Protocol::Cmd cmd, const char* data,
                            SocketPtr socket_ptr) const
{
    const_cast<MessageHandler&>(*this).Handle(cmd, data, socket_ptr);
}

NAMESPACE_CPPNAT_END