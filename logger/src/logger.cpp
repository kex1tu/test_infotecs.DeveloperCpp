// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "logger.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string_view>

#include "logger_error.hpp"

namespace logger {

LoggerError Logger::log_message(LogLevel level,
                                std::string_view message) noexcept {
  try {
    if (!sink_) {
      return LoggerError::kNotInitialized;
    }

    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(min_level_)) {
      return LoggerError::kSuccess;
    }
    if (message.empty()) {
      return LoggerError::kInvalidArgument;
    }
    const std::string formatted = format_message(level, message);
    if (formatted.empty()) {
      return LoggerError::kSinkError;
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
    sink_->close();
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
  thread_local std::time_t cached_sec = 0;
  thread_local std::array<char, 24> cached_prefix{};
  if (time_t_now != cached_sec) {
    std::tm tm_buf{};
    localtime_r(&time_t_now, &tm_buf);
    cached_sec = time_t_now;
    std::strftime(cached_prefix.data(), cached_prefix.size(),
                  "%Y-%m-%d %H:%M:%S.", &tm_buf);
  }
  const auto ms_val = static_cast<uint32_t>(ms.count());
  std::array<char, 24> full_ts = cached_prefix;
  full_ts[20] = static_cast<char>('0' + (ms_val / 100));
  full_ts[21] = static_cast<char>('0' + ((ms_val / 10) % 10));
  full_ts[22] = static_cast<char>('0' + (ms_val % 10));
  result.append(full_ts.data(), 23);
}

}  // namespace
std::string Logger::format_message(LogLevel level,
                                   std::string_view message) noexcept {
  try {
    std::string result;

    // резервируем 64 байта под префикс [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL]
    // плюс длину сообщения, чтобы избежать повторных реаллокаций памяти.
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
