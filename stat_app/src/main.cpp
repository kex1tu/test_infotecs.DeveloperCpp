// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include <atomic>
#include <csignal>
#include <iostream>

#include "stats_collector.hpp"
#include "tcp_server.hpp"
#include "utilities.hpp"

namespace {

std::atomic<stats::TcpServer*> server_ptr{nullptr};

void handle_sigint(int /*signal*/) noexcept {
  if (auto* server = server_ptr.load()) {
    server->stop();
  }
}

}  // namespace
int main(int argc, char* argv[]) {
  const auto config = stats::parse_cli_args(argc, argv);
  if (!config) {
    stats::print_usage((argv != nullptr && argv[0] != nullptr) ? argv[0]
                                                               : "stat_app");
    return 1;
  }

  stats::StatsCollector collector;
  stats::TcpServer server(*config, collector);
  const bool started = server.start();
  if (!started) {
    std::cerr << "Failed to start server on " << config->address << ":"
              << config->port << '\n';
    return 1;
  }

  // чтобы при ctrl + c не было утечек и процеес нормальо завершался
  server_ptr.store(&server);
  std::signal(SIGINT, handle_sigint);
  std::signal(SIGTERM, handle_sigint);

  std::cout << "Server started on " << config->address << ":" << config->port
            << " (N=" << config->n_messages << ", T=" << config->timeout_sec
            << "s)\n";

  server.run();
  server_ptr.store(nullptr);
  std::cout << "Server stopped\n";
  return 0;
}
