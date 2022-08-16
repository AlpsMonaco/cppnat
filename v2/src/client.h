#ifndef __CPPNAT_CLIENT_H__
#define __CPPNAT_CLIENT_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

class Client
{
public:
    Client(const std::string& server_addr, std::uint16_t server_port,
           const std::string& proxy_addr, std::uint16_t proxy_port);
    ~Client();

    void Start();

protected:
    asio::io_service ios_;
    asio::ip::tcp::endpoint server_endpoint_;
    asio::ip::tcp::endpoint proxy_endpoint_;
};

NAMESPACE_CPPNAT_END

#endif