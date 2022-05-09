#ifndef __CPP_NAT_CONNECTION_H
#define __CPP_NAT_CONNECTION_H
#include <vector>
#include <vector>

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