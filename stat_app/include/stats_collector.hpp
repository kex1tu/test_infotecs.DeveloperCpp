// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iosfwd>
#include <limits>

#include "log_level.hpp"

namespace stats {

struct StatsSnapshot {
  uint64_t total_count_{0};
  uint64_t debug_count_{0};
  uint64_t info_count_{0};
  uint64_t warning_count_{0};
  uint64_t error_count_{0};
  uint64_t unknown_count_{0};
  uint64_t last_hour_count_{0};

  uint64_t min_length_{0};
  uint64_t max_length_{0};
  double avg_length_{0.0};
  bool operator==(const StatsSnapshot& other) const noexcept {
    return total_count_ == other.total_count_ &&
           debug_count_ == other.debug_count_ &&
           info_count_ == other.info_count_ &&
           warning_count_ == other.warning_count_ &&
           error_count_ == other.error_count_ &&
           unknown_count_ == other.unknown_count_ &&
           last_hour_count_ == other.last_hour_count_ &&
           min_length_ == other.min_length_ &&
           max_length_ == other.max_length_ &&
           std::abs(avg_length_ - other.avg_length_) < 1e-6;
  }

  bool operator!=(const StatsSnapshot& other) const noexcept {
    return !(*this == other);
  }
};

/**
 * \brief класс сбора и агрегации статистики сообщений лога.
 */
class StatsCollector {
 public:
  StatsCollector() noexcept;
  ~StatsCollector() = default;

  StatsCollector(const StatsCollector&) = delete;
  StatsCollector& operator=(const StatsCollector&) = delete;
  StatsCollector(StatsCollector&&) noexcept = default;
  StatsCollector& operator=(StatsCollector&&) noexcept = default;

  void add_message(logger::LogLevel level, uint64_t length) noexcept;

  [[nodiscard]] StatsSnapshot get_snapshot() noexcept;

  [[nodiscard]] uint64_t total_count() const noexcept;

  static void print_stats(const StatsSnapshot& s, std::ostream& out) noexcept;
  static void print_stats(const StatsSnapshot& s) noexcept;

 private:
  void clean_old_timestamps(std::chrono::steady_clock::time_point now) noexcept;

  uint64_t total_count_{0};
  uint64_t debug_count_{0};
  uint64_t info_count_{0};
  uint64_t warning_count_{0};
  uint64_t error_count_{0};
  uint64_t unknown_count_{0};

  uint64_t min_length_{std::numeric_limits<uint64_t>::max()};
  uint64_t max_length_{0};
  uint64_t total_length_sum_{0};
  std::deque<std::chrono::steady_clock::time_point> timestamps_;
};

}  // namespace stats