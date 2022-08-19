#include "log.h"

NAMESPACE_CPPNAT_START

void Log::SetLogLevel(spdlog::level::level_enum level) {
  Ins().LogLevel(level);
};

Log::Log() : log_ptr_(spdlog::basic_logger_mt("cppnat", "cppnat.log")) {
  log_ptr_->set_level(spdlog::level::debug);
  log_ptr_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  log_ptr_->flush_on(spdlog::level::debug);
  spdlog::set_level(spdlog::level::debug);
}

Log& Log::Ins() {
  static Log logger;
  return logger;
}

void Log::LogLevel(spdlog::level::level_enum level) {
  log_ptr_->set_level(level);
  spdlog::set_level(level);
}

void HandleErrorCode(const std::error_code& ec) {
  Log::Error("{} {}", ec.value(), ec.message());
}

NAMESPACE_CPPNAT_END