// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <cstdint>
#include <string_view>

namespace logger {

/**
 * \brief Уровни важности сообщений логирования.
 */
enum class [[nodiscard]] LogLevel : std::uint8_t {
  kDebug = 0,    ///< Отладочная информация
  kInfo = 1,     ///< Информационные сообщения
  kWarning = 2,  ///< Предупреждения
  kError = 3,    ///< Ошибки
  kUnknown = 4,  ///< Неизвестный уровень
};

/**
 * \brief Конвертирует уровень лога в строковое представление.
 *
 * \param[in] level Уровень логирования.
 * \return Строковое представление уровня лога.
 */
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

/**
 * \brief Конвертирует символ в верхний регистр.
 *
 * \param[in] c Символ.
 * \return Символ в верхнем регистре.
 */
constexpr char to_upper_ascii(char c) noexcept {
  return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A')) : c;
}

/**
 * \brief Проверяет равенство строк без учета регистра.
 *
 * \param[in] a Первая строка.
 * \param[in] b Вторая строка.
 * \return true, если строки равны без учета регистра, иначе false.
 */
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

/**
 * \brief Разбирает строковое представление уровня логирования без учета
 * регистра.
 *
 * \param[in] str Строка, содержащая уровень лога.
 * \param[out] out_level Переменная для записи распарсенного уровня лога.
 * \return true, если уровень лога успешно распознан, иначе false.
 * \post При возврате true out_level содержит валидный LogLevel.
 */
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