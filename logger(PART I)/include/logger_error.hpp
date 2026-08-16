// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.
#pragma once

#include <cstdint>
#include <string_view>

namespace logger {

/**
 * \brief Коды ошибок подсистемы логирования.
 */
enum class [[nodiscard]] LoggerError : std::uint8_t {
  kSuccess = 0,          ///< Успешное выполнение
  kNotInitialized = 1,   ///< Логгер не инициализирован
  kInvalidArgument = 2,  ///< Некорректный аргумент
  kSinkError = 3,        ///< Ошибка приемника
};

/**
 * \brief Преобразует код ошибки логгера в строковое представление.
 *
 * \param[in] err Код ошибки логгера.
 * \return Строковое представление ошибки.
 */
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