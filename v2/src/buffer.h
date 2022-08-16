#ifndef __CPPNAT_BUFFER_H__
#define __CPPNAT_BUFFER_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

class Buffer
{
public:
    Buffer();
    ~Buffer();

protected:
    char buffer_[Protocol::max_size];

public:
    operator auto()
    {
        return asio::buffer(buffer_, Protocol::max_size);
    }
};

NAMESPACE_CPPNAT_END

#endif