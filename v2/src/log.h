#ifndef __CPP_NAT_LOG_H__
#define __CPP_NAT_LOG_H__

#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "prefix.h"

NAMESPACE_CPPNAT_START

class Log {
 public:
  static void SetLogLevel(spdlog::level::level_enum level) {
    Ins().LogLevel(level);
  };

  template <typename... Args>
  static void Error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    Ins().log_ptr_->error(fmt, std::forward<Args>(args)...);
    spdlog::error(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void Info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    Ins().log_ptr_->info(fmt, std::forward<Args>(args)...);
    spdlog::info(fmt, std::forward<Args>(args)...);
  }

 protected:
  using LogPtr = std::shared_ptr<spdlog::logger>;
  Log() : log_ptr_(spdlog::basic_logger_mt("cppnat", "cppnat.log")) {
    log_ptr_->set_level(spdlog::level::debug);
    log_ptr_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    log_ptr_->flush_on(spdlog::level::debug);
    spdlog::set_level(spdlog::level::debug);
  }

  static Log& Ins() {
    Log logger;
    return logger;
  }

  void LogLevel(spdlog::level::level_enum level) {
    log_ptr_->set_level(level);
    spdlog::set_level(level);
  }

  LogPtr log_ptr_;
};

NAMESPACE_CPPNAT_END

#endif
