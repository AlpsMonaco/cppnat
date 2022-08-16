#ifndef __CPPNAT_SESSION_H__
#define __CPPNAT_SESSION_H__

#include "prefix.h"

NAMESPACE_CPPNAT_START

class Session
    : public std::enable_shared_from_this<Session>
{
public:
    Session(asio::ip::tcp::socket& socket);
    ~Session();

    void Start();

protected:
    static constexpr size_t buffer_size_ = Protocol::max_size;

    asio::ip::tcp::socket socket_;
    size_t read_size_;
    size_t extra_offset_;
    char buffer_[buffer_size_];

    void MessageLoop();
    void ParseBuffer();
    void HandleBuffer(Protocol::Header header, const char* body);
    void ParseExtraBuffer();
};

NAMESPACE_CPPNAT_END

#endif