// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace logger {

enum class [[nodiscard]] LogLevel : uint8_t {
  kDebug = 0,
  kInfo = 1,
  kWarning = 2,
  kError = 3,
  kUnknown = 4,
};

[[nodiscard]] constexpr std::string_view log_level_to_string(
    LogLevel level) noexcept {
  switch (level) {
    case LogLevel::kDebug:
      return "DEBUG";
    case LogLevel::kInfo:
      return "INFO";
    case LogLevel::kWarning:
      return "WARNING";
    case LogLevel::kError:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

namespace helpers {

constexpr char to_upper_ascii(char c) noexcept {
  return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A')) : c;
}

constexpr bool is_equal(std::string_view a, std::string_view b) noexcept {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (to_upper_ascii(a[i]) != to_upper_ascii(b[i])) {
      return false;
    }
  }
  return true;
}

}  // namespace helpers

constexpr bool parse_log_level(std::string_view str,
                               LogLevel& out_level) noexcept {
  if (helpers::is_equal(str, "DEBUG")) {
    out_level = LogLevel::kDebug;
    return true;
  }
  if (helpers::is_equal(str, "INFO")) {
    out_level = LogLevel::kInfo;
    return true;
  }
  if (helpers::is_equal(str, "WARNING")) {
    out_level = LogLevel::kWarning;
    return true;
  }
  if (helpers::is_equal(str, "ERROR")) {
    out_level = LogLevel::kError;
    return true;
  }
  if (helpers::is_equal(str, "UNKNOWN")) {
    out_level = LogLevel::kUnknown;
    return true;
  }
  return false;
}

}  // namespace logger