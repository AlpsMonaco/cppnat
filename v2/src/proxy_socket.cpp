#include "proxy_socket.h"

NAMESPACE_CPPNAT_START

ProxySocket::ProxySocket(SocketPtr socket_ptr, size_t id)
    : socket_ptr_(socket_ptr),
      proxy_data_()
{
    proxy_data_.id = id;
}

ProxySocket::~ProxySocket()
{
}

ProxySocketMgr::ProxySocketMgr()
{
}

ProxySocketMgr::~ProxySocketMgr()
{
}

NAMESPACE_CPPNAT_END