// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.
// NOLINTBEGIN
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "stats_collector.hpp"
#include "tcp_server.hpp"
#include "tests.hpp"
#include "utilities.hpp"

namespace {

std::optional<stats::ServerConfig> run_parse_cli(
    const std::vector<std::string>& args) noexcept {
  std::vector<char*> argv;
  argv.reserve(args.size());
  for (const auto& arg : args) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  return stats::parse_cli_args(static_cast<int>(argv.size()), argv.data());
}

}  // namespace

bool test_1() {
  uint16_t port = 0;
  ASSERT_TRUE(stats::parse_port("1", port));
  ASSERT_EQ(port, 1);
  ASSERT_TRUE(stats::parse_port("8080", port));
  ASSERT_EQ(port, 8080);
  ASSERT_TRUE(stats::parse_port("65535", port));
  ASSERT_EQ(port, 65535);

  ASSERT_TRUE(!stats::parse_port("0", port));
  ASSERT_TRUE(!stats::parse_port("65536", port));
  ASSERT_TRUE(!stats::parse_port("-1", port));
  ASSERT_TRUE(!stats::parse_port("abc", port));
  ASSERT_TRUE(!stats::parse_port("", port));
  return true;
}

bool test_2() {
  uint64_t val = 0;
  ASSERT_TRUE(stats::parse_positive_uint64("1", val));
  ASSERT_EQ(val, 1ULL);

  ASSERT_TRUE(stats::parse_positive_uint64("100", val));
  ASSERT_EQ(val, 100ULL);

  ASSERT_TRUE(!stats::parse_positive_uint64("0", val));
  ASSERT_TRUE(!stats::parse_positive_uint64("-5", val));
  ASSERT_TRUE(!stats::parse_positive_uint64("xyz", val));
  ASSERT_TRUE(!stats::parse_positive_uint64("", val));
  return true;
}

bool test_3() {
  ASSERT_TRUE(stats::is_valid_ipv4_address("127.0.0.1"));
  ASSERT_TRUE(stats::is_valid_ipv4_address("0.0.0.0"));
  ASSERT_TRUE(stats::is_valid_ipv4_address("192.168.0.1"));
  ASSERT_TRUE(stats::is_valid_ipv4_address("255.255.255.255"));

  ASSERT_TRUE(!stats::is_valid_ipv4_address("256.0.0.1"));
  ASSERT_TRUE(!stats::is_valid_ipv4_address("127.0.0"));
  ASSERT_TRUE(!stats::is_valid_ipv4_address("abc"));
  ASSERT_TRUE(!stats::is_valid_ipv4_address(""));
  ASSERT_TRUE(!stats::is_valid_ipv4_address("127.0.0.1.1"));
  return true;
}

bool test_4() {
  auto cfg = run_parse_cli({"./stat_app", "127.0.0.1", "8080", "10", "5"});
  ASSERT_TRUE(cfg.has_value());
  ASSERT_EQ(cfg->address, "127.0.0.1");
  ASSERT_EQ(cfg->port, 8080);
  ASSERT_EQ(cfg->n_messages, 10ULL);
  ASSERT_EQ(cfg->timeout_sec, 5ULL);

  auto cfg2 = run_parse_cli({"./stat_app", "0.0.0.0", "9000", "1", "1"});
  ASSERT_TRUE(cfg2.has_value());
  ASSERT_EQ(cfg2->address, "0.0.0.0");
  ASSERT_EQ(cfg2->port, 9000);
  ASSERT_EQ(cfg2->n_messages, 1ULL);
  ASSERT_EQ(cfg2->timeout_sec, 1ULL);
  return true;
}

bool test_5() {
  ASSERT_TRUE(!run_parse_cli({"./stat_app"}).has_value());
  ASSERT_TRUE(!run_parse_cli({"./stat_app", "127.0.0.1"}).has_value());
  ASSERT_TRUE(
      !run_parse_cli({"./stat_app", "127.0.0.1", "8080", "10"}).has_value());
  ASSERT_TRUE(
      !run_parse_cli({"./stat_app", "127.0.0.1", "8080", "10", "5", "extra"})
           .has_value());

  ASSERT_TRUE(!run_parse_cli({"./stat_app", "invalid_ip", "8080", "10", "5"})
                   .has_value());

  ASSERT_TRUE(!run_parse_cli({"./stat_app", "127.0.0.1", "70000", "10", "5"})
                   .has_value());
  ASSERT_TRUE(!run_parse_cli({"./stat_app", "127.0.0.1", "abc", "10", "5"})
                   .has_value());

  ASSERT_TRUE(!run_parse_cli({"./stat_app", "127.0.0.1", "8080", "0", "5"})
                   .has_value());
  ASSERT_TRUE(!run_parse_cli({"./stat_app", "127.0.0.1", "8080", "10", "0"})
                   .has_value());
  return true;
}

bool test_6() {
  stats::StatsCollector collector;
  auto snap = collector.get_snapshot();
  ASSERT_EQ(snap.total_count_, 0ULL);
  ASSERT_EQ(snap.debug_count_, 0ULL);
  ASSERT_EQ(snap.info_count_, 0ULL);
  ASSERT_EQ(snap.warning_count_, 0ULL);
  ASSERT_EQ(snap.error_count_, 0ULL);
  ASSERT_EQ(snap.unknown_count_, 0ULL);
  ASSERT_EQ(snap.last_hour_count_, 0ULL);
  ASSERT_EQ(snap.min_length_, 0ULL);
  ASSERT_EQ(snap.max_length_, 0ULL);
  ASSERT_EQ(snap.avg_length_, 0.0);
  return true;
}

bool test_7() {
  stats::StatsCollector collector;
  collector.add_message(logger::LogLevel::kDebug, 10);
  collector.add_message(logger::LogLevel::kInfo, 20);
  collector.add_message(logger::LogLevel::kWarning, 30);
  collector.add_message(logger::LogLevel::kError, 40);
  collector.add_message(logger::LogLevel::kUnknown, 50);

  auto snap = collector.get_snapshot();
  ASSERT_EQ(snap.total_count_, 5ULL);
  ASSERT_EQ(snap.debug_count_, 1ULL);
  ASSERT_EQ(snap.info_count_, 1ULL);
  ASSERT_EQ(snap.warning_count_, 1ULL);
  ASSERT_EQ(snap.error_count_, 1ULL);
  ASSERT_EQ(snap.unknown_count_, 1ULL);
  ASSERT_EQ(snap.last_hour_count_, 5ULL);
  ASSERT_EQ(snap.min_length_, 10ULL);
  ASSERT_EQ(snap.max_length_, 50ULL);
  ASSERT_TRUE(std::abs(snap.avg_length_ - 30.0) < 0.001);

  std::ostringstream oss;
  stats::StatsCollector::print_stats(snap, oss);
  ASSERT_TRUE(!oss.str().empty());
  ASSERT_TRUE(oss.str().find("Total messages:     5") != std::string::npos);
  return true;
}

bool test_8() {
  auto entry1 = stats::parse_log_line(
      "[2026-08-17 19:00:00.123] [INFO] Service started successfully");
  ASSERT_TRUE(entry1.has_value());
  ASSERT_EQ(static_cast<int>(entry1->level),
            static_cast<int>(logger::LogLevel::kInfo));
  ASSERT_EQ(entry1->message_length, 28ULL);

  auto entry2 = stats::parse_log_line("[2026-08-17 19:00:00.123] [DEBUG] test");
  ASSERT_TRUE(entry2.has_value());
  ASSERT_EQ(static_cast<int>(entry2->level),
            static_cast<int>(logger::LogLevel::kDebug));
  ASSERT_EQ(entry2->message_length, 4ULL);

  auto entry3 = stats::parse_log_line("[2026-08-17] [WARNING] Warn msg");
  ASSERT_TRUE(entry3.has_value());
  ASSERT_EQ(static_cast<int>(entry3->level),
            static_cast<int>(logger::LogLevel::kWarning));
  ASSERT_EQ(entry3->message_length, 8ULL);

  auto entry4 = stats::parse_log_line("[2026-08-17] [ERROR] Err msg");
  ASSERT_TRUE(entry4.has_value());
  ASSERT_EQ(static_cast<int>(entry4->level),
            static_cast<int>(logger::LogLevel::kError));
  ASSERT_EQ(entry4->message_length, 7ULL);
  auto entry5 = stats::parse_log_line("[2026-08-17] [INFO]");
  ASSERT_TRUE(entry5.has_value());
  ASSERT_EQ(entry5->message_length, 0ULL);
  return true;
}

bool test_9() {
  ASSERT_TRUE(!stats::parse_log_line("").has_value());
  ASSERT_TRUE(!stats::parse_log_line("Hello world").has_value());
  ASSERT_TRUE(!stats::parse_log_line("[2026-08-17 19:00:00.123").has_value());
  ASSERT_TRUE(
      !stats::parse_log_line("[2026-08-17 19:00:00.123] INFO msg").has_value());
  ASSERT_TRUE(!stats::parse_log_line("[2026-08-17 19:00:00.123] [TRACE] msg")
                   .has_value());
  ASSERT_TRUE(
      !stats::parse_log_line("[2026-08-17 19:00:00.123] [] msg").has_value());
  return true;
}

bool test_10() {
  stats::UniqueFd empty_fd;
  ASSERT_TRUE(!empty_fd.is_valid());
  ASSERT_EQ(empty_fd.get(), -1);

  stats::UniqueFd fd1(999);
  ASSERT_TRUE(fd1.is_valid());
  ASSERT_EQ(fd1.get(), 999);

  stats::UniqueFd fd2(std::move(fd1));
  ASSERT_TRUE(!fd1.is_valid());
  ASSERT_EQ(fd1.get(), -1);
  ASSERT_TRUE(fd2.is_valid());
  ASSERT_EQ(fd2.get(), 999);

  const int raw = fd2.release();
  ASSERT_EQ(raw, 999);
  ASSERT_TRUE(!fd2.is_valid());
  ASSERT_EQ(fd2.get(), -1);

  stats::UniqueFd fd3(888);
  stats::UniqueFd fd4;
  fd4 = std::move(fd3);
  ASSERT_TRUE(!fd3.is_valid());
  ASSERT_TRUE(fd4.is_valid());
  ASSERT_EQ(fd4.get(), 888);

  fd4.reset();
  ASSERT_TRUE(!fd4.is_valid());
  return true;
}

bool test_11() {
  stats::StatsSnapshot snap1;
  stats::StatsSnapshot snap2;
  ASSERT_TRUE(snap1 == snap2);
  ASSERT_TRUE(!(snap1 != snap2));
  snap1.total_count_ = 10;
  ASSERT_TRUE(snap1 != snap2);
  ASSERT_TRUE(!(snap1 == snap2));
  snap2.total_count_ = 10;
  ASSERT_TRUE(snap1 == snap2);
  snap1.last_hour_count_ = 5;
  ASSERT_TRUE(snap1 != snap2);
  snap2.last_hour_count_ = 5;
  ASSERT_TRUE(snap1 == snap2);
  snap1.avg_length_ = 15.5;
  ASSERT_TRUE(snap1 != snap2);
  snap2.avg_length_ = 15.5;
  ASSERT_TRUE(snap1 == snap2);
  return true;
}

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Тесты stat_app\n";
  std::cout << "==========================================\n";
  RUN_TEST(test_1);
  RUN_TEST(test_2);
  RUN_TEST(test_3);
  RUN_TEST(test_4);
  RUN_TEST(test_5);
  RUN_TEST(test_6);
  RUN_TEST(test_7);
  RUN_TEST(test_8);
  RUN_TEST(test_9);
  RUN_TEST(test_10);
  RUN_TEST(test_11);
  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}
// NOLINTEND