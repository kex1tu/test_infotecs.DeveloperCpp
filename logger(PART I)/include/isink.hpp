// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <string_view>

#include "logger_error.hpp"

namespace logger {

/**
 * \brief Абстрактный интерфейс приемника (sink) сообщений логирования.
 *
 * Определяет интерфейс для вывода форматированных логов в целевые каналы
 * (файлы, сокеты, память). Владение приемником передается объекту
 * Logger через std::unique_ptr.
 *
 * \note Не является потокобезопасным. Потокобезопасность обеспечивается
 *       вызывающей стороной.
 */
class ISink {
 public:
  virtual ~ISink() = default;

  /**
   * \brief Записывает форматированное сообщение в целевой приемник.
   *
   * \param[in] formatted_message Полностью сформированная строка лога.
   * \return LoggerError::kSuccess при успешной записи,
   *         LoggerError::kSinkError при ошибке вывода или закрытом приемнике.
   * \pre is_open() == true
   * \pre !formatted_message.empty()
   */
  virtual LoggerError write(std::string_view formatted_message) noexcept = 0;

  /**
   * \brief Закрывает приемник и освобождает все связанные ресурсы.
   *
   * \post is_open() == false
   */
  virtual void close() noexcept = 0;

  /**
   * \brief Проверяет, открыт ли приемник.
   *
   * \return true, если приемник открыт и готов принимать сообщения,
   *         false, если приемник закрыт или находится в состоянии ошибки.
   */
  virtual bool is_open() const noexcept = 0;

 protected:
  ISink(const ISink&) = delete;
  ISink& operator=(const ISink&) = delete;
  ISink(ISink&&) = delete;
  ISink& operator=(ISink&&) = delete;

  ISink() = default;
};

}  // namespace logger
