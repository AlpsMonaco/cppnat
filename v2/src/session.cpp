#include "session.h"

NAMESPACE_CPPNAT_START

Session::Session(asio::ip::tcp::socket& socket)
    : socket_(std::move(socket)),
      read_size_(0),
      extra_offset_(0),
      buffer_()
{
}

Session::~Session()
{
}

void Session::Start()
{
    MessageLoop();
}

void Session::MessageLoop()
{
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(buffer_ + read_size_, buffer_size_ - read_size_),
                            [self, this](const std::error_code& ec,
                                         std::size_t length) -> void
                            {
                                if (ec)
                                {
                                    HandleError(ec);
                                    return;
                                }
                                read_size_ += length;
                                ParseBuffer();
                                MessageLoop();
                            });
}

void Session::ParseBuffer()
{
    if (read_size_ < Protocol::header_size)
        return;
    auto header = Protocol::GetHeader(buffer_);
    if (read_size_ < header.packet_size)
        return;
    if (read_size_ == header.packet_size)
        read_size_ = 0;
    else
        extra_offset_ = header.packet_size;
    HandleBuffer(header, buffer_ + Protocol::header_size);
    if (extra_offset_ > 0)
        ParseExtraBuffer();
}

void Session::ParseExtraBuffer()
{
    size_t remain_size = 0;
    do
    {
        remain_size = read_size_ - extra_offset_;
        if (remain_size < Protocol::header_size)
        {
            memcpy(buffer_, buffer_ + extra_offset_, remain_size);
            read_size_ = remain_size;
            extra_offset_ = 0;
            return;
        }
        auto header = Protocol::GetHeader(buffer_ + extra_offset_);
        if (remain_size < header.packet_size)
        {
            memcpy(buffer_, buffer_ + extra_offset_, remain_size);
            read_size_ = remain_size;
            extra_offset_ = 0;
            return;
        }
        HandleBuffer(header, buffer_ + extra_offset_ + Protocol::header_size);
        if (remain_size == header.packet_size)
        {
            read_size_ = 0;
            extra_offset_ = 0;
        }
        else
            extra_offset_ += header.packet_size;
    } while (extra_offset_ > 0);
}

void Session::HandleBuffer(Protocol::Header header, const char* body)
{
}

NAMESPACE_CPPNAT_END