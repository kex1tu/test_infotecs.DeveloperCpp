// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <string>
#include <string_view>

#include "isink.hpp"
#include "logger_error.hpp"

namespace logger {

/**
 * \brief Приемник логов, передающий сообщения по сети через TCP-сокет.
 *
 * Реализует интерфейс ISink для отправки логов на удаленный сервер.
 */
class SocketSink final : public ISink {
 public:
  SocketSink() = default;
  ~SocketSink() override;

  /**
   * \brief Подключается к удаленному серверу по указанному адресу и порту.
   *
   * \param[in] address IPv4-адрес сервера в виде строки.
   * \param[in] port Порт сервера [1..65535].
   * \return LoggerError::kSuccess при успешном подключении,
   *         LoggerError::kInvalidArgument при некорректном адресе или порте,
   *         LoggerError::kSinkError при ошибке подключения.
   * \pre is_open() == false
   * \post is_open() == true при успешном подключении.
   */
  LoggerError connect(const std::string& address, int port) noexcept;

  /**
   * \brief Записывает форматированное сообщение в сокет.
   *
   * \param[in] formatted_message Полностью сформированная строка лога.
   * \return LoggerError::kSuccess при успешной записи,
   *         LoggerError::kSinkError при ошибке записи или закрытом приемнике.
   * \pre is_open() == true
   * \pre !formatted_message.empty()
   */
  LoggerError write(std::string_view formatted_message) noexcept override;

  /**
   * \brief Закрывает сокет и освобождает ресурсы.
   *
   * \post is_open() == false
   */
  void close() noexcept override;

  /**
   * \brief Проверяет, открыт ли сокет.
   *
   * \return true, если сокет открыт и готов принимать сообщения,
   *         false, если сокет закрыт или находится в состоянии ошибки.
   */
  bool is_open() const noexcept override;

 private:
  int socket_fd_ = -1;
};

}  // namespace logger
