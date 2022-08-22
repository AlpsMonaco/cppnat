#ifndef __CPP_NAT_LOG_H__
#define __CPP_NAT_LOG_H__

#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "prefix.h"

NAMESPACE_CPPNAT_START

class Log
{
public:
    static void SetLogLevel(spdlog::level::level_enum level);

    template <typename... Args>
    static void Error(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        Ins().log_ptr_->error(fmt, std::forward<Args>(args)...);
        spdlog::error(fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    static void Error(const T& msg)
    {
        Ins().log_ptr_->error(msg);
        spdlog::error(msg);
    }

    template <typename... Args>
    static void Info(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        Ins().log_ptr_->info(fmt, std::forward<Args>(args)...);
        spdlog::info(fmt, std::forward<Args>(args)...);
    }

    static void Bytes(const std::string_view& sv,
                      const std::string& comment = "")
    {
        Ins().log_ptr_->debug("{} size:{} {}", comment, sv.size(),
                              spdlog::to_hex(sv.begin(), sv.end()));
    }

    static void Bytes(const char* bytes, size_t size,
                      const std::string& comment = "")
    {
        Bytes(std::string_view(bytes, size), comment);
    }

    static void SocketEvent(const std::string& event, SocketPtr socket_ptr)
    {
        Ins().log_ptr_->info("{} {}:{}", event,
                             socket_ptr->remote_endpoint().address().to_string(),
                             socket_ptr->remote_endpoint().port());
    }

    static void SocketErrorEvent(const std::string& event, SocketPtr socket_ptr,
                                 const std::error_code& ec)
    {
        Ins().log_ptr_->error("{} {}:{} {}", event,
                              socket_ptr->remote_endpoint().address().to_string(),
                              socket_ptr->remote_endpoint().port(), ec.message());
    }
    static void SetLogName(const std::string& log_name);

protected:
    using LogPtr = std::shared_ptr<spdlog::logger>;
    Log(const std::string& log_name);
    static Log& Ins(const std::string& log_name = "cppnat");
    void LogLevel(spdlog::level::level_enum level);
    LogPtr log_ptr_;
};

void HandleErrorCode(const std::error_code&);

NAMESPACE_CPPNAT_END

#endif
