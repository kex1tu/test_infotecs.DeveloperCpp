// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "socket_sink.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace logger {

SocketSink::~SocketSink() { close(); }

LoggerError SocketSink::connect(const std::string& address, int port,
                                int send_timeout_sec) noexcept {
  if (address.empty() || port <= 0 || port > 65535 || send_timeout_sec <= 0) {
    return LoggerError::kInvalidArgument;
  }

  address_ = address;
  port_ = port;
  send_timeout_sec_ = send_timeout_sec;

  return reconnect();
}

LoggerError SocketSink::reconnect() noexcept {
  socket_fd_.reset();

  if (address_.empty() || port_ <= 0 || port_ > 65535 ||
      send_timeout_sec_ <= 0) {
    return LoggerError::kInvalidArgument;
  }

  const int raw_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (raw_fd < 0) {
    return LoggerError::kSinkError;
  }
  UniqueFd sock(raw_fd);

  // Таймаут на отправку SO_SNDTIMEO гарантирует, что рабочий поток логирования
  // не зависнет бесконечно в send()
  timeval timeout{};
  timeout.tv_sec = send_timeout_sec_;
  timeout.tv_usec = 0;
  if (::setsockopt(sock.get(), SOL_SOCKET, SO_SNDTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
    return LoggerError::kSinkError;
  }

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(static_cast<uint16_t>(port_));

  if (::inet_pton(AF_INET, address_.c_str(), &server_addr.sin_addr) <= 0) {
    return LoggerError::kInvalidArgument;
  }

  if (::connect(sock.get(), reinterpret_cast<struct sockaddr*>(&server_addr),
                sizeof(server_addr)) < 0) {
    return LoggerError::kSinkError;
  }

  socket_fd_ = std::move(sock);
  return LoggerError::kSuccess;
}

LoggerError SocketSink::write(std::string_view formatted_message) noexcept {
  if (!socket_fd_.is_valid()) {
    return LoggerError::kSinkError;
  }

  const char* data = formatted_message.data();
  std::size_t remaining = formatted_message.size();
  bool reconnected = false;
  // флаг MSG_NOSIGNAL нужен, он подавляет генерацию сигнала SIGPIPE, значит
  // приложение не останавливается
  while (remaining > 0) {
    const ssize_t sent =
        ::send(socket_fd_.get(), data, remaining, MSG_NOSIGNAL);
    if (sent <= 0) {
      if (errno == EINTR) {
        continue;
      }
      close();
      if (reconnected || reconnect() != LoggerError::kSuccess) {
        return LoggerError::kSinkError;
      }
      reconnected = true;
      // если переподключились сбрасываем указатель и остаток чтобы отправить
      // зананво полностью
      data = formatted_message.data();
      remaining = formatted_message.size();
      continue;
    }
    data += sent;
    remaining -= static_cast<std::size_t>(sent);
  }

  return LoggerError::kSuccess;
}

void SocketSink::close() noexcept { socket_fd_.reset(); }

bool SocketSink::is_open() const noexcept { return socket_fd_.is_valid(); }

}  // namespace logger
