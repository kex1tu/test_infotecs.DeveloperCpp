// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "tcp_server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "stats_collector.hpp"
#include "utilities.hpp"

namespace stats {

/**
 * \brief сессия соединения (слушающий сокет сервера или клиент).
 */
struct Session {
  UniqueFd socket_fd;
  std::string stream_buffer;
  bool is_listener = false;
};

struct TcpServer::Impl {
  ServerConfig config_;
  StatsCollector& collector_;
  std::vector<Session> sessions_;
  std::vector<pollfd> poll_fds_;
  std::atomic<bool> is_running_{false};
  std::chrono::steady_clock::time_point last_print_time_;
  StatsSnapshot last_printed_snapshot_{};

  Impl(ServerConfig config, StatsCollector& collector) noexcept
      : config_(std::move(config)), collector_(collector) {}

  ~Impl() {
    stop();
    clean_resources();
  }

  bool start() noexcept {
    UniqueFd server_fd(::socket(AF_INET, SOCK_STREAM, 0));
    if (!server_fd.is_valid()) {
      return false;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(config_.port);
    if (::inet_pton(AF_INET, config_.address.c_str(),
                    &server_address.sin_addr) <= 0) {
      return false;
    }

    int opt = 1;
    ::setsockopt(server_fd.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (::bind(server_fd.get(),
               reinterpret_cast<const struct sockaddr*>(&server_address),
               sizeof(server_address)) < 0) {
      return false;
    }

    if (::listen(server_fd.get(), SOMAXCONN) < 0) {
      return false;
    }

    clean_resources();
    const int fd = server_fd.get();
    try {
      poll_fds_.push_back({fd, POLLIN, 0});
      sessions_.emplace_back(Session{std::move(server_fd), "", true});
    } catch (...) {
      // на всякий случай чистим за собой в случае исключения
      if (poll_fds_.size() > sessions_.size()) {
        poll_fds_.pop_back();
      }
      if (sessions_.size() > poll_fds_.size()) {
        sessions_.pop_back();
      }
      return false;
    }

    is_running_ = true;
    return true;
  }

  void run() noexcept {
    if (!is_running_ || sessions_.empty() || poll_fds_.empty()) {
      return;
    }
    last_print_time_ = std::chrono::steady_clock::now();
    last_printed_snapshot_ = collector_.get_snapshot();

    while (is_running_) {
      const int ready_count =
          ::poll(poll_fds_.data(), static_cast<nfds_t>(poll_fds_.size()), 100);
      if (ready_count > 0) {
        for (std::size_t i = 0; i < poll_fds_.size(); ++i) {
          if ((poll_fds_[i].revents &
               (POLLIN | POLLHUP | POLLERR | POLLNVAL)) != 0) {
            if (sessions_[i].is_listener) {
              handle_new_connection();
            } else {
              const bool alive = handle_client_data(i);
              if (!alive) {
                sessions_[i].socket_fd.reset();
                continue;
              }
              process_client_buffer(sessions_[i]);
            }
          }
        }
        for (std::size_t i = sessions_.size(); i > 0; --i) {
          if (!sessions_[i - 1].socket_fd.is_valid()) {
            close_session(i - 1);
          }
        }
      }

      const auto now = std::chrono::steady_clock::now();
      const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                               now - last_print_time_)
                               .count();
      if (elapsed >= static_cast<int64_t>(config_.timeout_sec)) {
        if (collector_.get_snapshot() != last_printed_snapshot_) {
          print_statistics();
        }
      }
    }

    clean_resources();
  }

  void stop() noexcept { is_running_ = false; }

  void handle_new_connection() noexcept {
    if (sessions_.empty() || !sessions_[0].is_listener) {
      return;
    }
    sockaddr_in client_address{};
    socklen_t client_address_len = sizeof(client_address);
    UniqueFd client_socket(::accept(
        sessions_[0].socket_fd.get(),
        reinterpret_cast<sockaddr*>(&client_address), &client_address_len));
    if (!client_socket.is_valid()) {
      return;
    }
    try {
      poll_fds_.push_back({client_socket.get(), POLLIN, 0});
      sessions_.emplace_back(Session{std::move(client_socket), "", false});
      // NOLINTNEXTLINE чтобы не было предупреждения о пустом catch
    } catch (...) {
      // на всякий случай чистим за собой в случае исключения
      if (poll_fds_.size() > sessions_.size()) {
        poll_fds_.pop_back();
      }
      if (sessions_.size() > poll_fds_.size()) {
        sessions_.pop_back();
      }
    }
  }

  bool handle_client_data(std::size_t index) noexcept {
    if (index >= sessions_.size()) {
      return false;
    }
    Session& client = sessions_[index];
    std::array<char, 4096> buffer{};
    const ssize_t bytes_read =
        ::read(client.socket_fd.get(), buffer.data(), buffer.size());
    if (bytes_read <= 0) {
      return false;
    }
    try {
      // чтобы не переполнять буфер
      if (client.stream_buffer.size() > kMaxStreamBufferSize) {
        client.socket_fd.reset();
        return false;
      }
      client.stream_buffer.append(buffer.data(),
                                  static_cast<std::size_t>(bytes_read));
    } catch (...) {
      return false;
    }
    return true;
  }

  void process_client_buffer(Session& session) noexcept {
    while (true) {
      if (session.stream_buffer.empty()) {
        break;
      }

      const std::size_t pos_n = session.stream_buffer.find('\n');
      if (pos_n == std::string::npos) {
        break;
      }

      const std::string line = session.stream_buffer.substr(0, pos_n);
      session.stream_buffer.erase(0, pos_n + 1);
      if (line.empty()) {
        continue;
      }
      std::cout << line << '\n';

      const auto parsed = parse_log_line(line);
      if (!parsed.has_value()) {
        continue;
      }

      collector_.add_message(parsed->level, parsed->message_length);
      if (collector_.total_count() > 0 &&
          collector_.total_count() % config_.n_messages == 0) {
        print_statistics();
      }
    }
  }

  void close_session(std::size_t index) noexcept {
    if (index == 0 || index >= sessions_.size()) {
      return;
    }
    // swap-and-pop чтобы быстрее удалять
    std::swap(sessions_[index], sessions_.back());
    sessions_.pop_back();
    std::swap(poll_fds_[index], poll_fds_.back());
    poll_fds_.pop_back();
  }

  void clean_resources() noexcept {
    sessions_.clear();
    poll_fds_.clear();
  }

  void print_statistics() noexcept {
    const auto snapshot = collector_.get_snapshot();
    StatsCollector::print_stats(snapshot);
    last_printed_snapshot_ = snapshot;
    last_print_time_ = std::chrono::steady_clock::now();
  }
};

TcpServer::TcpServer(ServerConfig config, StatsCollector& collector) noexcept {
  try {
    pimpl_ = std::make_unique<Impl>(std::move(config), collector);
  } catch (...) {
    pimpl_ = nullptr;
  }
}

TcpServer::~TcpServer() = default;

TcpServer::TcpServer(TcpServer&&) noexcept = default;
TcpServer& TcpServer::operator=(TcpServer&&) noexcept = default;

bool TcpServer::start() noexcept { return pimpl_ ? pimpl_->start() : false; }

void TcpServer::run() noexcept {
  if (pimpl_) {
    pimpl_->run();
  }
}

void TcpServer::stop() noexcept {
  if (pimpl_) {
    pimpl_->stop();
  }
}

}  // namespace stats