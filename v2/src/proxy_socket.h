#ifndef __CPP_NAT_PROXY_SOCKET_H__
#define __CPP_NAT_PROXY_SOCKET_H__

#include "prefix.h"
#include "cs_message.h"

NAMESPACE_CPPNAT_START

class ProxySocket
{
public:
    ProxySocket(SocketPtr socket_ptr, size_t id);
    ~ProxySocket();

protected:
    SocketPtr socket_ptr_;
    ProxyData proxy_data_;
};

class ProxySocketMgr
{
public:
    ProxySocketMgr();
    ~ProxySocketMgr();

protected:
    std::map<size_t, ProxySocket> proxy_socket_map_;
};

NAMESPACE_CPPNAT_END

#endif