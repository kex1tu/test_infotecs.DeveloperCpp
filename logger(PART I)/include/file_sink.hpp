// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <fstream>
#include <string>
#include <string_view>

#include "isink.hpp"
#include "logger_error.hpp"

namespace logger {

/**
 * \brief Приемник логов, сохраняющий сообщения в локальный файл на диске.
 *
 * Реализует интерфейс ISink для сохранения логов в указанный файл.
 */
class FileSink final : public ISink {
 public:
  FileSink() = default;
  ~FileSink() override;

  /**
   * \brief Открывает файл с заданным именем для записи логов в режиме append.
   *
   * \param[in] filename Имя файла для открытия.
   * \return LoggerError::kSuccess при успешном открытии файла,
   *         LoggerError::kInvalidArgument если имя файла пустое,
   *         LoggerError::kSinkError при ошибке открытия файла.
   * \pre is_open() == false
   * \post is_open() == true при успешном открытии файла.
   */
  LoggerError open(const std::string& filename) noexcept;

  /**
   * \brief Записывает форматированное сообщение в файл.
   *
   * \param[in] formatted_message Полностью сформированная строка лога.
   * \return LoggerError::kSuccess при успешной записи,
   *         LoggerError::kSinkError при ошибке записи или закрытом приемнике.
   * \pre is_open() == true
   * \pre !formatted_message.empty()
   */
  LoggerError write(std::string_view formatted_message) noexcept override;

  /**
   * \brief Закрывает файл и освобождает ресурсы.
   *
   * \post is_open() == false
   */
  void close() noexcept override;

  /**
   * \brief Проверяет, открыт ли файл.
   *
   * \return true, если файл открыт и готов принимать сообщения,
   *         false, если файл закрыт или находится в состоянии ошибки.
   */
  bool is_open() const noexcept override;

 private:
  std::ofstream file_;
};

}  // namespace logger
