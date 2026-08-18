// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "stats_collector.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

namespace stats {

StatsCollector::StatsCollector() noexcept
    : min_length_(std::numeric_limits<uint64_t>::max()) {}

void StatsCollector::add_message(logger::LogLevel level,
                                 uint64_t length) noexcept {
  ++total_count_;
  total_length_sum_ += length;

  min_length_ = std::min(min_length_, length);
  max_length_ = std::max(max_length_, length);
  switch (level) {
    case logger::LogLevel::kDebug:
      ++debug_count_;
      break;
    case logger::LogLevel::kInfo:
      ++info_count_;
      break;
    case logger::LogLevel::kWarning:
      ++warning_count_;
      break;
    case logger::LogLevel::kError:
      ++error_count_;
      break;
    default:
      ++unknown_count_;
      break;
  }
  try {
    timestamps_.push_back(std::chrono::steady_clock::now());

    // NOLINTNEXTLINE
  } catch (...) {
  }
}

void StatsCollector::print_stats(const StatsSnapshot& s,
                                 std::ostream& out) noexcept {
  out << "\n========== STATS SNAPSHOT ==========\n"
      << "Total messages:     " << s.total_count_ << "\n"
      << "  DEBUG:            " << s.debug_count_ << "\n"
      << "  INFO:             " << s.info_count_ << "\n"
      << "  WARNING:          " << s.warning_count_ << "\n"
      << "  ERROR:            " << s.error_count_ << "\n"
      << "  UNKNOWN:          " << s.unknown_count_ << "\n"
      << "Last hour messages: " << s.last_hour_count_ << "\n"
      << "Message length (min/max/avg): " << s.min_length_ << " / "
      << s.max_length_ << " / " << s.avg_length_ << "\n"
      << "=====================================\n";
}

void StatsCollector::print_stats(const StatsSnapshot& s) noexcept {
  print_stats(s, std::cout);
}

StatsSnapshot StatsCollector::get_snapshot() noexcept {
  const auto now = std::chrono::steady_clock::now();
  clean_old_timestamps(now);

  StatsSnapshot snapshot;
  snapshot.total_count_ = total_count_;
  snapshot.debug_count_ = debug_count_;
  snapshot.info_count_ = info_count_;
  snapshot.warning_count_ = warning_count_;
  snapshot.error_count_ = error_count_;
  snapshot.unknown_count_ = unknown_count_;
  snapshot.last_hour_count_ = timestamps_.size();
  snapshot.min_length_ = (total_count_ > 0) ? min_length_ : 0;
  snapshot.max_length_ = (total_count_ > 0) ? max_length_ : 0;
  snapshot.avg_length_ = (total_count_ > 0)
                             ? static_cast<double>(total_length_sum_) /
                                   static_cast<double>(total_count_)
                             : 0.0;
  return snapshot;
}

uint64_t StatsCollector::total_count() const noexcept { return total_count_; }

void StatsCollector::clean_old_timestamps(
    std::chrono::steady_clock::time_point now) noexcept {
  const auto one_hour_ago = now - std::chrono::hours(1);
  while (!timestamps_.empty() && timestamps_.front() < one_hour_ago) {
    timestamps_.pop_front();
  }
}

}  // namespace stats
