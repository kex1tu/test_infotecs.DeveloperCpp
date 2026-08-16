// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "logger.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <string_view>

#include "logger_error.hpp"

namespace logger {

// Implementation of class Logger
// ////////////////////////////////
LoggerError Logger::log_message(LogLevel level,
                                std::string_view message) noexcept {
  try {
    if (!sink_) {
      return LoggerError::kNotInitialized;
    }
    // Фильтруем сообщения по минимальному уровню и валидируем входные параметры
    // до выполнения форматирования строки, чтобы исключить накладные расходы
    // на лишние аллокации памяти.

    if (static_cast<std::uint8_t>(level) <
        static_cast<std::uint8_t>(min_level_)) {
      return LoggerError::kSuccess;
    }
    if (message.empty()) {
      return LoggerError::kInvalidArgument;
    }
    const std::string formatted = format_message(level, message);

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

// Message formatting and timestamp helpers
// /////////////////////////////////////////

namespace {

void append_current_timestamp(std::string& result) {
  const auto now = std::chrono::system_clock::now();
  const auto time_t_now = std::chrono::system_clock::to_time_t(now);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;
  std::tm tm_buf{};
  localtime_r(&time_t_now, &tm_buf);

  std::array<char, 32> buf{};
  const int len = std::snprintf(
      buf.data(), buf.size(), "%04d-%02d-%02d %02d:%02d:%02d.%03lld",
      tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday, tm_buf.tm_hour,
      tm_buf.tm_min, tm_buf.tm_sec, static_cast<long long>(ms.count()));

  if (len > 0 && static_cast<std::size_t>(len) < buf.size()) {
    result.append(buf.data(), static_cast<size_t>(len));
  }
}

}  // namespace
std::string Logger::format_message(LogLevel level,
                                   std::string_view message) noexcept {
  try {
    std::string result;

    // Резервируем 64 байта под префикс [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL]
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
