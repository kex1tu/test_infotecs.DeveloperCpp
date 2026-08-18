// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "isink.hpp"
#include "log_level.hpp"
#include "logger_error.hpp"

namespace logger {

/**
 * \brief основной класс логгера, выполняющий форматирование и фильтрацию
 * сообщений.
 *
 * применяет фильтрацию по уровню логирования, формирует префикс с
 * датой/временем и направляет результат в установленный приемник ISink.

 * \note Не является потокобезопасным при параллельном вызове log_message().
 *       Для многопоточного использования применяйте в паре с ThreadSafeQueue.
 */
class Logger {
 public:
  explicit Logger(std::unique_ptr<ISink> sink,
                  LogLevel min_level = LogLevel::kDebug) noexcept
      : sink_(std::move(sink)), min_level_(min_level) {}

  ~Logger() noexcept { close(); }

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) noexcept = default;
  Logger& operator=(Logger&&) noexcept = default;

  /**
   * \brief записывает сообщение в лог, если его уровень не ниже порога
   * фильтрации.
   *
   * \param[in] level Уровень важности сообщения.
   * \param[in] message Сообщение для записи.
   * \return LoggerError::kSuccess при успешной записи или отсечении по уровню,
   *         LoggerError::kNotInitialized если приемник не установлен или
   * закрыт, LoggerError::kInvalidArgument если message пусто,
   *         LoggerError::kSinkError при ошибке записи в приемник.
   * \pre !message.empty()
   * \pre Приемник sink_ должен быть инициализирован и открыт.
   * \post При коде kSuccess сообщение гарантированно сброшено в целевой
   * приемник.
   */
  LoggerError log_message(LogLevel level, std::string_view message) noexcept;

  LoggerError set_level(LogLevel level) noexcept;
  LogLevel get_level() const noexcept;
  void close() noexcept;

 private:
  static std::string format_message(LogLevel level,
                                    std::string_view message) noexcept;

  std::unique_ptr<ISink> sink_;
  LogLevel min_level_{LogLevel::kDebug};
};

}  // namespace logger
