#include "client.h"

NAMESPACE_CPPNAT_START

Client::Client(const std::string& server_addr, std::uint16_t server_port,
               const std::string& proxy_addr, std::uint16_t proxy_port)
    : ios_(),
      server_endpoint_(asio::ip::address::from_string(server_addr),
                       server_port),
      proxy_endpoint_(asio::ip::address::from_string(proxy_addr),
                      proxy_port)
{
}

Client::~Client()
{
}

void Client::Start()
{
}

NAMESPACE_CPPNAT_END