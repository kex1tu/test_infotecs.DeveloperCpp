#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "log_level.hpp"
#include "logger_error.hpp"

namespace logger {
class ISink;
// логгер, можно использовать с файлом, сокетом или с чем-то что реализует
// интерфейс ISink
class Logger {
 public:
  Logger();
  ~Logger();

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  LoggerError init_with_file(const std::string& filename,
                             LogLevel min_level = LogLevel::kDebug) noexcept;

  LoggerError init_with_socket(const std::string& address, int port,
                               LogLevel min_level = LogLevel::kDebug) noexcept;

  LoggerError init_with_any_sink(
      std::unique_ptr<ISink> sink,
      LogLevel min_level = LogLevel::kDebug) noexcept;

  LoggerError log_message(LogLevel level, std::string_view message) noexcept;

  LoggerError set_level(LogLevel level) noexcept;

  LogLevel get_level() const noexcept;

  void close() noexcept;

 private:
  static std::string format_message(LogLevel level,
                                    std::string_view message) noexcept;

  std::unique_ptr<ISink> sink_;
  LogLevel min_level_{LogLevel::kDebug};
  bool initialized_{false};
};

}  // namespace logger
