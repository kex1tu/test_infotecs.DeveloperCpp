// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <cstdint>
#include <string_view>

namespace logger {

enum class [[nodiscard]] LoggerError : uint8_t {
  kSuccess = 0,
  kNotInitialized = 1,
  kInvalidArgument = 2,
  kSinkError = 3,
};

[[nodiscard]] constexpr std::string_view logger_error_to_string(
    LoggerError err) noexcept {
  switch (err) {
    case LoggerError::kSuccess:
      return "Success";
    case LoggerError::kNotInitialized:
      return "Logger not initialized";
    case LoggerError::kInvalidArgument:
      return "Invalid argument";
    case LoggerError::kSinkError:
      return "Sink error";
    default:
      return "Unknown error";
  }
}

}  // namespace logger