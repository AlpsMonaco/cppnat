#include "message_handler.h"

NAMESPACE_CPPNAT_START

MessageHandler::MessageHandler()
    : handler_map_()
{
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::Bind()
{
}

void MessageHandler::Handle(Protocol::Cmd cmd, const char* data)
{
}

NAMESPACE_CPPNAT_END