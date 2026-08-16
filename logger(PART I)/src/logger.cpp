#include "./../include/logger.hpp"

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string_view>

#include "./../include/file_sink.hpp"
#include "./../include/logger_error.hpp"
#include "./../include/socket_sink.hpp"

namespace logger {

Logger::Logger() = default;

Logger::~Logger() { close(); }

LoggerError Logger::init_with_file(const std::string& filename,
                                   LogLevel min_level) noexcept {
  auto sink = std::make_unique<FileSink>();
  const LoggerError err = sink->open(filename);
  if (err != LoggerError::kSuccess) {
    return err;
  }
  return init_with_any_sink(std::move(sink), min_level);
}

LoggerError Logger::init_with_socket(const std::string& address, int port,
                                     LogLevel min_level) noexcept {
  auto sink = std::make_unique<SocketSink>();
  const auto err = sink->connect(address, port);
  if (err != LoggerError::kSuccess) {
    return err;
  }
  return init_with_any_sink(std::move(sink), min_level);
}

LoggerError Logger::init_with_any_sink(std::unique_ptr<ISink> sink,
                                       LogLevel min_level) noexcept {
  if (initialized_) {
    return LoggerError::kAlreadyInitialized;
  }

  if (!sink || !sink->is_open()) {
    return LoggerError::kInvalidArgument;
  }

  sink_ = std::move(sink);
  min_level_ = min_level;
  initialized_ = true;

  return LoggerError::kSuccess;
}

LoggerError Logger::log_message(LogLevel level,
                                std::string_view message) noexcept {
  if (!initialized_ || !sink_) {
    return LoggerError::kNotInitialized;
  }

  if (static_cast<std::uint8_t>(level) <
      static_cast<std::uint8_t>(min_level_)) {
    return LoggerError::kSuccess;
  }

  const std::string formatted = format_message(level, message);

  return sink_->write(formatted);
}

LoggerError Logger::set_level(LogLevel level) noexcept {
  if (!initialized_) {
    return LoggerError::kNotInitialized;
  }
  min_level_ = level;
  return LoggerError::kSuccess;
}

LogLevel Logger::get_level() const noexcept { return min_level_; }

void Logger::close() noexcept {
  if (initialized_ && sink_) {
    initialized_ = false;
    sink_->close();
    sink_.reset();
  }
}

std::string Logger::format_message(LogLevel level,
                                   std::string_view message) noexcept {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  // надо поменять некрасиво выглядит
  std::tm tm_buf{};
  localtime_r(&time_t_now, &tm_buf);
  std::ostringstream oss;
  oss << '[' << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << '.'
      << std::setfill('0') << std::setw(3) << ms.count() << "] ["
      << log_level_to_string(level) << "] " << message << '\n';

  return oss.str();
}

}  // namespace logger
