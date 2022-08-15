#ifndef __CPPNAT_BUFFER_H__
#define __CPPNAT_BUFFER_H__

#include "message.h"

NAMESPACE_CPPNAT_START

class Buffer
{
public:
    Buffer();
    ~Buffer();

protected:
    char buffer_[Protocol::max_size];
};

NAMESPACE_CPPNAT_END

#endif