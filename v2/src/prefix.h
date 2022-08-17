#ifndef __CPPNAT_PREFIX_H__
#define __CPPNAT_PREFIX_H__

#define NAMESPACE_CPPNAT_START \
    namespace cppnat           \
    {

#define NAMESPACE_CPPNAT_END }

// C++ Standard library
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <string>

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

template <size_t buffer_size>
class BufferSize
{
public:
    Buffer() {}
    ~Buffer() {}
    static constexpr size_t size = buffer_size;

    char* Get() { return buffer_; }
    size_t Size() { return buffer_size; }

protected:
    buffer_[buffer_size];
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

using ErrorHandler = std::function<void(const std::error_code& ec)>;
using ErrorHandlerPtr = std::shared_ptr<ErrorHandler>;

NAMESPACE_CPPNAT_END

#endif