#ifndef __CPPNAT_MESSAGE_H__
#define __CPPNAT_MESSAGE_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

class Message
{
public:
    Message(const char *buffer);
    ~Message();

    Protocol::Size Size();
    Protocol::Cmd Cmd();
    const char *Data();

protected:
    const char *ptr_;
};

NAMESPACE_CPPNAT_END

#endif