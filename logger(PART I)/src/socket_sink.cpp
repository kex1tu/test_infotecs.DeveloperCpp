// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "socket_sink.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace logger {

// Implementation of class SocketSink
// //////////////////////////////////

SocketSink::~SocketSink() { close(); }

LoggerError SocketSink::connect(const std::string& address, int port) noexcept {
  // Закрываем предыдущий дескриптор при повторном подключении,
  // чтобы избежать утечки системных ресурсов
  if (socket_fd_ >= 0) {
    ::close(socket_fd_);
    socket_fd_ = -1;
  }

  if (address.empty() || port <= 0 || port > 65535) {
    return LoggerError::kInvalidArgument;
  }

  socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0) {
    return LoggerError::kSinkError;
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(static_cast<uint16_t>(port));

  if (::inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr) <= 0) {
    ::close(socket_fd_);
    socket_fd_ = -1;
    return LoggerError::kInvalidArgument;
  }

  if (::connect(socket_fd_, reinterpret_cast<struct sockaddr*>(&server_addr),
                sizeof(server_addr)) < 0) {
    ::close(socket_fd_);
    socket_fd_ = -1;
    return LoggerError::kSinkError;
  }

  return LoggerError::kSuccess;
}

LoggerError SocketSink::write(std::string_view formatted_message) noexcept {
  if (socket_fd_ < 0) {
    return LoggerError::kSinkError;
  }

  const char* data = formatted_message.data();
  std::size_t remaining = formatted_message.size();
  // Системный вызов POSIX send() может передать данные частично из-за
  // заполнения буфера отправки в стеке TCP/IP. Выполняем циклическую передачу
  // со сдвигом указателя, пока все байты formatted_message не будут полностью
  // отправлены в сокет.
  while (remaining > 0) {
    const ssize_t sent = ::send(socket_fd_, data, remaining, 0);
    if (sent <= 0) {
      return LoggerError::kSinkError;
    }
    data += sent;
    remaining -= static_cast<std::size_t>(sent);
  }

  return LoggerError::kSuccess;
}

void SocketSink::close() noexcept {
  if (socket_fd_ >= 0) {
    ::close(socket_fd_);
    socket_fd_ = -1;
  }
}

bool SocketSink::is_open() const noexcept { return socket_fd_ >= 0; }

}  // namespace logger
