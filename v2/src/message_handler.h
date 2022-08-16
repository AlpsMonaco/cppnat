#ifndef __CPPNAT_MESSAGE_HANDLER_H__
#define __CPPNAT_MESSAGE_HANDLER_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

class MessageWriter
{
public:
    MessageWriter() {}
    ~MessageWriter() {}

protected:
};

class MessageHandler
{
public:
    MessageHandler();
    ~MessageHandler();

    void Bind();
    void Handle(Protocol::Cmd cmd, const char* data);

protected:
    using Callback = std::function<void(const char*)>;

    std::map<Protocol::Cmd, Callback> handler_map_;
};

NAMESPACE_CPPNAT_END

#endif