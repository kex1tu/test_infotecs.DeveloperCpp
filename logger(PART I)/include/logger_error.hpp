#pragma once

#include <cstdint>

namespace logger {
// ошибки логгера
enum class LoggerError : std::uint8_t {
  kSuccess = 0,
  kFileOpenFailed = 1,
  kWriteFailed = 2,
  kSocketConnectFailed = 3,
  kSocketSendFailed = 4,
  kInvalidArgument = 5,
  kAlreadyInitialized = 6,
  kNotInitialized = 7,
};

constexpr const char* logger_error_to_string(LoggerError err) noexcept {
  switch (err) {
    case LoggerError::kSuccess:
      return "Success";
    case LoggerError::kFileOpenFailed:
      return "Failed to open log file";
    case LoggerError::kWriteFailed:
      return "Failed to write to log";
    case LoggerError::kSocketConnectFailed:
      return "Failed to connect to socket";
    case LoggerError::kSocketSendFailed:
      return "Failed to send data to socket";
    case LoggerError::kInvalidArgument:
      return "Invalid argument";
    case LoggerError::kAlreadyInitialized:
      return "Logger already initialized";
    case LoggerError::kNotInitialized:
      return "Logger not initialized";
  }
  return "Unknown error";
}

}  // namespace logger