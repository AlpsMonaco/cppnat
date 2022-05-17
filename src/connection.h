#ifndef __CPP_NAT_CONNECTION_H
#define __CPP_NAT_CONNECTION_H
#include <vector>
#include <memory>
#include <functional>

namespace cppnat
{
    template <typename Conn, typename Closer, size_t bufferSize>
    class BufferedConn
    {
    public:
        constexpr static size_t kBufferSize = bufferSize;
        using ConnId = size_t;
        using CloseFunction = std::function<void(Conn &)>;

        BufferedConn(ConnId id, Closer &&closeMethod) : id_(id),
                                                        pConn(nullptr),
                                                        closeFunction_(std::forward<Closer>(closeMethod))
        {
        }

        template <typename... Args>
        inline void Init(Args &&...args) { pConn_ = std::make_unique<Conn>(std::forward<Args>(args)...); }
        inline Conn &Conn() { return *pConn_; }
        inline ConnId Id() { return const_cast<BufferedConn *>(this)->Id(); }
        inline char *ReadBuffer() { return readBuffer_; }
        inline char *WriteBuffer() { return writeBuffer_; }
        inline void Close() { closeFunction_(*pConn_); }
        inline ConnId Id() const { return id_; }
        inline const char *ReadBuffer() const { return readBuffer_; }
        inline const char *WriteBuffer() const { return writeBuffer_; }
        inline size_t BufferSize() const { return kBufferSize; }
        inline size_t BufferSize() { return kBufferSize; }

    protected:
        CloseFunction closeFunction_;
        ConnId id_;
        std::unique_ptr<Conn> pConn_;
        char readBuffer_[kBufferSize];
        char writeBuffer_[kBufferSize];
    };

    template <typename Conn, size_t bufferSize, size_t maxSize>
    class BufferedConnMgr
    {
    public:
        using BufferedConnPtr = std::unique_ptr<
            BufferedConn<Conn, bufferSize>>;

        BufferedConnMgr() : activeConn_(0) {}

    protected:
        BufferedConnPtr bufferedConnList_[maxSize];
        size_t activeConn_;
    };

    template <typename Conn, size_t bufferSize>
    class ConnManager
    {
    public:
        struct ConnHelper
        {
            constexpr static size_t kBufferSize = bufferSize;
            size_t id;
            Conn *conn;
            char buffer[kBufferSize];
            char writeBuffer[kBufferSize];
            ConnHelper(size_t id) : id(id) {}
        };

        ConnManager(size_t num) : conns_(num, nullptr)
        {
        }
        ~ConnManager() {}

        template <typename... Args>
        ConnHelper *New(Args &&...args)
        {
            for (size_t i = 0; i < conns_.size(); i++)
            {
                if (conns_[i] == nullptr)
                {
                    conns_[i] = new ConnHelper(i);
                    conns_[i]->conn = new Conn(std::forward<Args>(args)...);
                    return conns_[i];
                }
                if (!conns_[i]->conn->is_open())
                {
                    *(conns_[i]->conn) = Conn(std::forward<Args>(args)...);
                    return conns_[i];
                }
            }
            return nullptr;
        }

        template <typename... Args>
        ConnHelper *NewAt(size_t index, Args &&...args)
        {
            if (index < conns_.size())
            {
                if (conns_[index] == nullptr)
                {
                    conns_[index] = new ConnHelper(index);
                    conns_[index]->conn = new Conn(std::forward<Args>(args)...);
                    return conns_[index];
                }
                if (!conns_[index]->conn->is_open())
                {
                    *(conns_[index]->conn) = Conn(std::forward<Args>(args)...);
                    return conns_[index];
                }
            }
            return nullptr;
        }

        void Set(size_t id, Conn *conn)
        {
            if (conns_[id] != nullptr)
            {
                delete conns_[id]->conn;
                conns_[id]->conn = conn;
            }
            else
            {
                conns_[id] = new ConnHelper(id);
                conns_[id]->conn = conn;
            }
        }

        ConnHelper *Get(size_t id)
        {
            assert(id < conns_.size());
            return conns_[id];
        }

    protected:
        std::vector<ConnHelper *> conns_;
    };
}

#endif