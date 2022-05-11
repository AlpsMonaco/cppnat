#ifndef __CPP_NAT_LOG_H__
#define __CPP_NAT_LOG_H__
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bin_to_hex.h>

namespace cppnat
{
#ifdef __NO_OUTPUT_TO_CONSOLE__
#define PRINT_BYTES(msg, bytes, len) \
    do                               \
    {                                \
    } while (0)
#define PRINT_INFO(msg) \
    do                  \
    {                   \
    } while (0)
#define PRINT_ERROR(msg) \
    do                   \
    {                    \
    } while (0)
#define PRINT_WARN(msg) \
    do                  \
    {                   \
    } while (0)
#define PRINT_DEBUG(msg) \
    do                   \
    {                    \
    } while (0)
#define PRINT_TRACE(msg) \
    do                   \
    {                    \
    } while (0)
#define PRINT_CRITICAL(msg) \
    do                      \
    {                       \
    } while (0)
#else
#define PRINT_BYTES(msg, sv)                                             \
    do                                                                   \
    {                                                                    \
        spdlog::info("{}{}", msg, spdlog::to_hex(sv.begin(), sv.end())); \
    } while (0)

#define PRINT_INFO(msg)    \
    do                     \
    {                      \
        spdlog::info(msg); \
    } while (0)
#define PRINT_ERROR(msg)    \
    do                      \
    {                       \
        spdlog::error(msg); \
    } while (0)
#define PRINT_WARN(msg)    \
    do                     \
    {                      \
        spdlog::warn(msg); \
    } while (0)
#define PRINT_DEBUG(msg)    \
    do                      \
    {                       \
        spdlog::debug(msg); \
    } while (0)
#define PRINT_TRACE(msg)    \
    do                      \
    {                       \
        spdlog::trace(msg); \
    } while (0)
#define PRINT_CRITICAL(msg)    \
    do                         \
    {                          \
        spdlog::critical(msg); \
    } while (0)
#endif

    class Log
    {
    public:
        using Logger = std::shared_ptr<spdlog::logger>;

        static Logger &Ins()
        {
            static Log log;
            return log.pLogger_;
        }

        inline void Flush() { pLogger_->flush(); }

    protected:
        Log() : pLogger_(spdlog::basic_logger_mt("cppnat", "cppnat.log"))
        {
            pLogger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
            pLogger_->flush_on(spdlog::level::err);
        }
        ~Log() {}

        Logger pLogger_;
    };

#define LOG_BYTES(msg, arr, arr_length)                                            \
    do                                                                             \
    {                                                                              \
        const auto &m = msg;                                                       \
        std::string_view sv(arr, arr_length);                                      \
        PRINT_BYTES(m, sv);                                                        \
        cppnat::Log::Ins()->info("{}{}", m, spdlog::to_hex(sv.begin(), sv.end())); \
    } while (0)

#define LOG_INFO(message)            \
    do                               \
    {                                \
        const auto &m = message;     \
        PRINT_INFO(m);               \
        cppnat::Log::Ins()->info(m); \
    } while (0)

#define LOG_ERROR(message)            \
    do                                \
    {                                 \
        const auto &m = message;      \
        PRINT_ERROR(m);               \
        cppnat::Log::Ins()->error(m); \
    } while (0)

#define LOG_WARN(message)            \
    do                               \
    {                                \
        const auto &m = message;     \
        PRINT_WARN(m);               \
        cppnat::Log::Ins()->warn(m); \
    } while (0)

#define LOG_DEBUG(message)            \
    do                                \
    {                                 \
        const auto &m = message;      \
        PRINT_DEBUG(m);               \
        cppnat::Log::Ins()->debug(m); \
    } while (0)

#define LOG_TRACE(message)            \
    do                                \
    {                                 \
        const auto &m = message;      \
        PRINT_TRACE(m);               \
        cppnat::Log::Ins()->trace(m); \
    } while (0)

#define LOG_CRITICAL(message)            \
    do                                   \
    {                                    \
        const auto &m = message;         \
        PRINT_CRITICAL(m);               \
        cppnat::Log::Ins()->critical(m); \
    } while (0)
}

#endif