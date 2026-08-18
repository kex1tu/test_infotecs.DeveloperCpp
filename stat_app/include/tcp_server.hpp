// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace stats {
constexpr std::size_t kMaxStreamBufferSize = 64ULL * 1024ULL;  // 64 KB
class StatsCollector;

/**
 * \brief параметры конфигурации TCP-сервера сбора статистики.
 */
struct ServerConfig {
  std::string address = "127.0.0.1";
  uint16_t port = 8080;
  uint64_t n_messages = 10;
  uint64_t timeout_sec = 5;
};

/**
 * \brief TCP-сервер сбора статистики логов.
 */
class TcpServer {
 public:
  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;
  TcpServer(TcpServer&&) noexcept;
  TcpServer& operator=(TcpServer&&) noexcept;
  ~TcpServer();

  explicit TcpServer(ServerConfig config, StatsCollector& collector) noexcept;

  [[nodiscard]] bool start() noexcept;

  void run() noexcept;

  void stop() noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace stats