#pragma once

#include <string>
#include <string_view>

#include "logger_error.hpp"
#include "sink.hpp"

namespace logger {
// для работы с сокетом
class SocketSink final : public ISink {
 public:
  SocketSink() = default;
  ~SocketSink() override;

  SocketSink(const SocketSink&) = delete;
  SocketSink& operator=(const SocketSink&) = delete;
  SocketSink(SocketSink&&) = delete;
  SocketSink& operator=(SocketSink&&) = delete;

  LoggerError connect(const std::string& address, int port) noexcept;
  LoggerError write(std::string_view formatted_message) noexcept override;
  void close() noexcept override;
  bool is_open() const noexcept override;

 private:
  int socket_fd_ = -1;
};

}  // namespace logger
