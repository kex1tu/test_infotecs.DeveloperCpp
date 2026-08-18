// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <string_view>

#include "logger_error.hpp"

namespace logger {

/**
 * \brief абстрактный интерфейс приемника (isink) сообщений логирования.
 *
 * определяет интерфейс для вывода форматированных логов в целевые каналы
 * (файлы, сокеты, память). владение приемником передается объекту
 * Logger через std::unique_ptr.
 *
 * \note не является потокобезопасным. потокобезопасность обеспечивается
 *       вызывающей стороной.
 */
class ISink {
 public:
  virtual ~ISink() = default;
  /**
   * \brief записывает готовое форматированное сообщение в целевой канал.
   *
   * \return LoggerError::kSuccess при успешной записи,
   *         LoggerError::kSinkError при сбое I/O или если канал закрыт.
   */
  virtual LoggerError write(std::string_view formatted_message) noexcept = 0;
  /**
   * \brief закрывает канал вывода и освобождает системные ресурсы (файлы,
   * сокеты).
   */
  virtual void close() noexcept = 0;
  /**
   * \brief проверяет, готов ли приемник к записи сообщений.
   * \return true, если канал открыт и валиден, иначе false.
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
