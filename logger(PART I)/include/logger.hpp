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
 * \brief Основной класс логгера, выполняющий форматирование и фильтрацию
 * сообщений.
 *
 * Передает отформатированные строки в установленный приемник ISink.
 *
 * \note Не является потокобезопасным при параллельном вызове log_message().
 *       Для многопоточного использования применяйте в паре с ThreadSafeQueue.
 */
class Logger {
 public:
  /**
   * \brief Создает логгер с заданным приемником и минимальным порогом
   * фильтрации.
   *
   * \param[in] sink Уникальный указатель на реализацию ISink.
   * \param[in] min_level Минимальный порог фильтрации сообщений.
   */
  explicit Logger(std::unique_ptr<ISink> sink,
                  LogLevel min_level = LogLevel::kDebug) noexcept
      : sink_(std::move(sink)), min_level_(min_level) {}

  ~Logger() noexcept { close(); }

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) noexcept = default;
  Logger& operator=(Logger&&) noexcept = default;

  /**
   * \brief Записывает сообщение в лог, если его уровень не ниже порога
   * фильтрации.
   *
   * \param[in] level Уровень важности сообщения.
   * \param[in] message Сообщение для записи.
   * \return LoggerError::kSuccess при успешной записи или отсечении по уровню,
   *         LoggerError::kNotInitialized если приемник не установлен или
   * закрыт, LoggerError::kInvalidArgument если message пусто,
   *         LoggerError::kSinkError при ошибке записи в приемник.
   * \pre !message.empty()
   */
  LoggerError log_message(LogLevel level, std::string_view message) noexcept;

  /**
   * \brief Устанавливает минимальный уровень фильтрации сообщений.
   *
   * \param[in] level Новый уровень логирования.
   * \return LoggerError::kSuccess.
   */
  LoggerError set_level(LogLevel level) noexcept;

  /**
   * \brief Возвращает текущий уровень фильтрации лога.
   *
   * \return Текущий LogLevel.
   */
  LogLevel get_level() const noexcept;

  /**
   * \brief Закрывает логгер и освобождает внутренний приемник.
   *
   * \post sink_ сброшен в nullptr.
   */
  void close() noexcept;

 private:
  static std::string format_message(LogLevel level,
                                    std::string_view message) noexcept;

  std::unique_ptr<ISink> sink_;
  LogLevel min_level_{LogLevel::kDebug};
};

}  // namespace logger
