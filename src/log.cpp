#include "log.h"

NAMESPACE_CPPNAT_START

void Log::SetLogLevel(spdlog::level::level_enum level) {
  Ins().LogLevel(level);
};

Log::Log(const std::string& log_name)
    : log_ptr_(spdlog::basic_logger_mt(log_name, log_name + ".log")) {
  log_ptr_->set_level(spdlog::level::info);
  log_ptr_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  log_ptr_->flush_on(spdlog::level::debug);
  spdlog::set_level(spdlog::level::info);
}

Log& Log::Ins(const std::string& log_name) {
  static Log logger(log_name);
  return logger;
}

void Log::LogLevel(spdlog::level::level_enum level) {
  log_ptr_->set_level(level);
  spdlog::set_level(level);
}

void HandleErrorCode(const std::error_code& ec) {
  Log::Error("{} {}", ec.value(), ec.message());
}

void Log::SetLogName(const std::string& log_name) { Ins(log_name); }

NAMESPACE_CPPNAT_END