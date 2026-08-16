#pragma once

#include <cstdint>
#include <string_view>
namespace logger {
// ошибки логгера
// nodiscard чтобы точно не забыть проверить возвращаемое значение
enum class [[nodiscard]] LoggerError : std::uint8_t {
  kSuccess = 0,
  kNotInitialized = 1,
  kAlreadyInitialized = 2,
  kInvalidArgument = 3,
  kSinkError = 4,
};
// переход к string_view чтобы более единообразно выгляжел код и был чуть
// быстрее засчёт хранения размера
[[nodiscard]] constexpr std::string_view logger_error_to_string(
    LoggerError err) noexcept {
  switch (err) {
    case LoggerError::kSuccess:
      return "Success";
    case LoggerError::kNotInitialized:
      return "Logger not initialized";
    case LoggerError::kAlreadyInitialized:
      return "Logger already initialized";
    case LoggerError::kInvalidArgument:
      return "Invalid argument";
    case LoggerError::kSinkError:
      return "Sink error";
    default:
      return "Unknown error";
  }
}
}  // namespace logger