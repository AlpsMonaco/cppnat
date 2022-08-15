#include "message.h"

NAMESPACE_CPPNAT_START

Message::Message(const char *buffer)
    : ptr_(buffer)
{
}

Message::~Message()
{
}

Protocol::Size Message::Size()
{
}

NAMESPACE_CPPNAT_END