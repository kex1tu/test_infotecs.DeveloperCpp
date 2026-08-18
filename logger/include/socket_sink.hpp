// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <string>
#include <string_view>

#include "isink.hpp"
#include "logger_error.hpp"
#include "unique_fd.hpp"

namespace logger {

class SocketSink final : public ISink {
 public:
  SocketSink() = default;
  ~SocketSink() override;
  /**
   * \brief устанавливает TCP-соединение с удаленным сервером сбора логов.
   *
   * создает сокет IPv4
   *
   * \param[in] address строковый IPv4-адрес сервера (например, "127.0.0.1").
   * \param[in] port номер TCP-порта в диапазоне [1..65535].
   * \return LoggerError::kSuccess — соединение успешно установлено;
   *         LoggerError::kInvalidArgument — передан пустой/невалидный IP или
   * port вне диапазона [1..65535]; LoggerError::kSinkError — ошибка системных
   * вызовов socket() или connect().
   *
   * \pre !address.empty() && port >= 1 && port <= 65535
   * \post при успешном коде возврата is_open() == true.
   */
  LoggerError connect(const std::string& address, int port) noexcept;
  LoggerError write(std::string_view formatted_message) noexcept override;
  void close() noexcept override;
  bool is_open() const noexcept override;

 private:
  LoggerError reconnect() noexcept;

  UniqueFd socket_fd_;
  std::string address_;
  int port_ = 0;
};

}  // namespace logger
