#include "logger.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <string_view>

#include "logger_error.hpp"

namespace logger {

// фикс, если были какие-то ошибки при форматировании то возрващает пустую
// строку, надо возрващать LoggerError
LoggerError Logger::log_message(LogLevel level,
                                std::string_view message) noexcept {
  try {
    if (!sink_) {
      return LoggerError::kNotInitialized;
    }

    if (static_cast<std::uint8_t>(level) <
        static_cast<std::uint8_t>(min_level_)) {
      return LoggerError::kSuccess;
    }

    const std::string formatted = format_message(level, message);
    if (formatted.empty()) {
      return LoggerError::kInvalidArgument;
    }
    return sink_->write(formatted);
  } catch (...) {
    return LoggerError::kSinkError;
  }
}

LoggerError Logger::set_level(LogLevel level) noexcept {
  min_level_ = level;
  return LoggerError::kSuccess;
}

LogLevel Logger::get_level() const noexcept { return min_level_; }

void Logger::close() noexcept {
  if (sink_) {
    try {
      sink_->close();
      // NOLINTNEXTLINE
    } catch (...) {
    }
    sink_.reset();
  }
}
namespace {

void append_current_timestamp(std::string& result) {
  const auto now = std::chrono::system_clock::now();
  const auto time_t_now = std::chrono::system_clock::to_time_t(now);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;
  std::tm tm_buf{};
// в тз сказано что целевой системой будет ubuntu/debian, но на всякий сделаю.
// чтобы была хоть какая-то переносимость
#if defined(_WIN32) || defined(_WIN64)
  localtime_s(&tm_buf, &time_t_now);
#else
  localtime_r(&time_t_now, &tm_buf);
#endif
  std::array<char, 32> tm_no_ms_str{};
  std::strftime(tm_no_ms_str.data(), tm_no_ms_str.size(), "%Y-%m-%d %H:%M:%S",
                &tm_buf);
  std::array<char, 48> tm_full{};

  const int len =
      std::snprintf(tm_full.data(), tm_full.size(), "%s.%03lld",
                    tm_no_ms_str.data(), static_cast<long long>(ms.count()));
  if (len > 0 && static_cast<std::size_t>(len) < tm_full.size()) {
    result.append(tm_full.data(), static_cast<size_t>(len));
  }
}

}  // namespace
std::string Logger::format_message(LogLevel level,
                                   std::string_view message) noexcept {
  try {
    std::string result;
    result.reserve(64 + message.size());
    result += "[";
    append_current_timestamp(result);
    result += "] [";
    result += log_level_to_string(level);
    result += "] ";
    result += message;
    result += '\n';
    return result;
  } catch (...) {
    return std::string();
  }
}

}  // namespace logger
