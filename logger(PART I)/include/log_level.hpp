#pragma once
#include <cstdint>
#include <string_view>

namespace logger {
// уровни лога
enum class [[nodiscard]] LogLevel : std::uint8_t {
  kDebug = 0,
  kInfo = 1,
  kWarning = 2,
  kError = 3,
  kUnknown = 4,
};
// чтобы не терять и структура была как в logger error
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
namespace {
constexpr char to_upper_ascii(char c) noexcept {
  return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A')) : c;
}
}  // namespace

// toupper не является constexpr, поэтому и is_equal не является constexpr
// поэтомц напишем свой аналог
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
// switch case чтобы быстрее сравнивать строки
constexpr bool parse_log_level(std::string_view str,
                               LogLevel& out_level) noexcept {
  if (str.empty()) {
    return false;
  }

  switch (to_upper_ascii(str.front())) {
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