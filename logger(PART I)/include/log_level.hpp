#pragma once

#include <cctype>
#include <cstdint>
#include <string_view>

namespace logger {
// уровни лога
enum class LogLevel : std::uint8_t {
  kDebug = 0,
  kInfo = 1,
  kWarning = 2,
  kError = 3,
  kUnknown = 4,
};

constexpr const char* log_level_to_string(LogLevel level) noexcept {
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

constexpr bool is_equal(std::string_view a, std::string_view b) noexcept {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (toupper(a[i]) != toupper(b[i])) {
      return false;
    }
  }
  return true;
}

constexpr bool parse_log_level(std::string_view str,
                               LogLevel& out_level) noexcept {
  if (str.empty()) {
    return false;
  }

  switch (toupper(str.front())) {
    case 'D':
      if (is_equal(str, "DEBUG")) {
        out_level = LogLevel::kDebug;
        return true;
      }
      break;
    case 'I':
      if (is_equal(str, "INFO")) {
        out_level = LogLevel::kInfo;
        return true;
      }
      break;
    case 'W':
      if (is_equal(str, "WARNING")) {
        out_level = LogLevel::kWarning;
        return true;
      }
      break;
    case 'E':
      if (is_equal(str, "ERROR")) {
        out_level = LogLevel::kError;
        return true;
      }
      break;
    case 'U':
      if (is_equal(str, "UNKNOWN")) {
        out_level = LogLevel::kUnknown;
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

}  // namespace logger