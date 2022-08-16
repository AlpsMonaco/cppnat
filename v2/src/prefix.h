#ifndef __CPPNAT_PREFIX_H__
#define __CPPNAT_PREFIX_H__

#define NAMESPACE_CPPNAT_START \
    namespace cppnat           \
    {

#define NAMESPACE_CPPNAT_END }

// C++ Standard library
#include <cstdint>
#include <string>
#include <memory>
#include <iostream>
#include <map>
#include <functional>

// include only third_party directory to compiler's include path parameter.
#include <asio/asio/include/asio.hpp>

NAMESPACE_CPPNAT_START

class Protocol
{
public:
    using Size = std::uint16_t;
    using Cmd = std::uint16_t;

    static constexpr Size header_size = sizeof(Size) + sizeof(Cmd);
    static constexpr Size max_size = ~0;
    static constexpr Size data_size = max_size - header_size;

    struct Header
    {
        Size packet_size;
        Cmd cmd;
    };

    static Header GetHeader(const char* buffer)
    {
        return *reinterpret_cast<const Header*>(buffer);
    }
};

using SocketPtr = std::shared_ptr<asio::ip::tcp::socket>;
inline SocketPtr CreateSocket(asio::io_service& ios)
{
    return std::make_shared<asio::ip::tcp::socket>(ios);
}

inline void HandleError(const asio::error_code& ec)
{
    std::cerr << ec << std::endl;
}

using TimerPtr = std::shared_ptr<asio::steady_timer>;
inline TimerPtr CreateTimer(asio::io_service& ios)
{
    return std::make_shared<asio::steady_timer>(ios);
}

NAMESPACE_CPPNAT_END

#endif