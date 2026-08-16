#pragma once

#include <memory>
#include <string>
#include <utility>

#include "file_sink.hpp"
#include "log_level.hpp"
#include "logger.hpp"
#include "logger_error.hpp"
#include "socket_sink.hpp"

namespace logger {

// фабрика файлового логгера
inline std::pair<LoggerError, std::unique_ptr<Logger>> make_file_logger(
    const std::string& filename,
    LogLevel min_level = LogLevel::kDebug) noexcept {
  try {
    auto sink = std::make_unique<FileSink>();
    const LoggerError err = sink->open(filename);
    if (err != LoggerError::kSuccess) {
      return {err, nullptr};
    }

    auto log = std::make_unique<Logger>(std::move(sink), min_level);
    return {LoggerError::kSuccess, std::move(log)};
  } catch (...) {
    return {LoggerError::kSinkError, nullptr};
  }
}

// фабрика сетевого логгера
inline std::pair<LoggerError, std::unique_ptr<Logger>> make_socket_logger(
    const std::string& address, int port,
    LogLevel min_level = LogLevel::kDebug) noexcept {
  try {
    auto sink = std::make_unique<SocketSink>();
    const LoggerError err = sink->connect(address, port);
    if (err != LoggerError::kSuccess) {
      return {err, nullptr};
    }

    auto log = std::make_unique<Logger>(std::move(sink), min_level);
    return {LoggerError::kSuccess, std::move(log)};
  } catch (...) {
    return {LoggerError::kSinkError, nullptr};
  }
}

// фабрика с произвольным ISink
inline std::pair<LoggerError, std::unique_ptr<Logger>> make_custom_logger(
    std::unique_ptr<ISink> sink,
    LogLevel min_level = LogLevel::kDebug) noexcept {
  try {
    if (!sink || !sink->is_open()) {
      return {LoggerError::kInvalidArgument, nullptr};
    }

    auto log = std::make_unique<Logger>(std::move(sink), min_level);
    return {LoggerError::kSuccess, std::move(log)};
  } catch (...) {
    return {LoggerError::kInvalidArgument, nullptr};
  }
}

}  // namespace logger
