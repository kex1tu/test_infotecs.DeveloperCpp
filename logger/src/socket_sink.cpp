// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "socket_sink.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace logger {

SocketSink::~SocketSink() { close(); }

LoggerError SocketSink::connect(const std::string& address, int port) noexcept {
  if (address.empty() || port <= 0 || port > 65535) {
    return LoggerError::kInvalidArgument;
  }

  address_ = address;
  port_ = port;

  return reconnect();
}

LoggerError SocketSink::reconnect() noexcept {
  socket_fd_.reset();

  if (address_.empty() || port_ <= 0 || port_ > 65535) {
    return LoggerError::kInvalidArgument;
  }

  const int raw_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (raw_fd < 0) {
    return LoggerError::kSinkError;
  }
  UniqueFd sock(raw_fd);

  struct sockaddr_in server_addr {};
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
  bool reconected = false;
  // флаг MSG_NOSIGNAL нужен, он подавляет генерацию сигнала SIGPIPE, значит
  // приложение не останавливается
  while (remaining > 0) {
    const ssize_t sent =
        ::send(socket_fd_.get(), data, remaining, MSG_NOSIGNAL);
    if (sent <= 0) {
      close();
      if (reconected || reconnect() != LoggerError::kSuccess) {
        return LoggerError::kSinkError;
      }
      reconected = true;
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
