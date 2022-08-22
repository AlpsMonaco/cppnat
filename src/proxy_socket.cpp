#include "proxy_socket.h"

NAMESPACE_CPPNAT_START

ProxySocket::ProxySocket(std::uint16_t id, SocketPtr socket_ptr)
    : proxy_data_(),
      socket_ptr_(socket_ptr),
      on_recv_(),
      on_read_error_(),
      on_write_error_()
{
    proxy_data_.id = id;
    proxy_data_.data_size = 0;
}

ProxySocket::~ProxySocket() {}

void ProxySocket::ReadOnce()
{
    auto self(shared_from_this());
    socket_ptr_->async_read_some(
        asio::buffer(proxy_data_.data, proxy_data_.writable_size),
        [this, self](const std::error_code& ec, size_t length) -> void
        {
            if (ec)
            {
                on_read_error_(proxy_data_.id, ec);
                return;
            }
            Log::Bytes(proxy_data_.data, length, "recv from proxy data");
            proxy_data_.data_size = length;
            on_recv_(proxy_data_);
        });
}

void ProxySocket::SetOnRecv(const OnRecvCallback& callback)
{
    on_recv_ = callback;
}

void ProxySocket::SetOnReadError(const OnReadErrorCallback& callback)
{
    on_read_error_ = callback;
}

void ProxySocket::SetOnWriteError(const OnWriteErrorCallback& callback)
{
    on_write_error_ = callback;
}

void ProxySocket::Write(DynamicBufferPtr buffer_ptr)
{
    auto self(shared_from_this());
    Log::Bytes(buffer_ptr->Get(), buffer_ptr->Size(), "write to proxy socket");
    asio::async_write(
        *socket_ptr_, asio::buffer(buffer_ptr->Get(), buffer_ptr->Size()),
        [buffer_ptr, self, this](const std::error_code& ec, size_t) -> void
        {
            if (ec)
            {
                on_write_error_(proxy_data_.id, ec);
                return;
            }
        });
}

void ProxySocket::Close()
{
    if (socket_ptr_->is_open()) socket_ptr_->close();
}

NAMESPACE_CPPNAT_END