#ifndef __CPP_NAT_CONNECTION_H
#define __CPP_NAT_CONNECTION_H
#include <vector>
#include <memory>
#include <functional>

namespace cppnat
{
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