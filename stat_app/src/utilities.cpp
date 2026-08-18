// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include "utilities.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>

#include <charconv>
#include <iostream>

namespace stats {

std::optional<ParsedLogEntry> parse_log_line(std::string_view line) noexcept {
  if (line.empty()) {
    return std::nullopt;
  }
  const std::size_t first_close = line.find(']');
  if (first_close == std::string_view::npos) {
    return std::nullopt;
  }
  const std::size_t second_open = line.find('[', first_close + 1);
  if (second_open == std::string_view::npos) {
    return std::nullopt;
  }
  const std::size_t second_close = line.find(']', second_open + 1);
  if (second_close == std::string_view::npos) {
    return std::nullopt;
  }

  const std::string_view level_str =
      line.substr(second_open + 1, second_close - second_open - 1);

  logger::LogLevel level{};
  if (!logger::parse_log_level(level_str, level)) {
    return std::nullopt;
  }

  const std::size_t msg_start = line.find_first_not_of(" \t", second_close + 1);
  const std::size_t msg_len =
      (msg_start != std::string_view::npos) ? line.size() - msg_start : 0;

  return ParsedLogEntry{level, msg_len};
}

bool parse_port(std::string_view str, uint16_t& port) noexcept {
  if (str.empty()) {
    return false;
  }
  int val = 0;
  const auto [ptr, err] =
      std::from_chars(str.data(), str.data() + str.size(), val);
  if (err == std::errc{} && ptr == str.data() + str.size() && val >= 1 &&
      val <= 65535) {
    port = static_cast<uint16_t>(val);
    return true;
  }
  return false;
}

bool parse_positive_uint64(std::string_view str, uint64_t& out_val) noexcept {
  if (str.empty()) {
    return false;
  }
  uint64_t val = 0;
  const auto [ptr, err] =
      std::from_chars(str.data(), str.data() + str.size(), val);
  if (err == std::errc{} && ptr == str.data() + str.size() && val >= 1) {
    out_val = val;
    return true;
  }
  return false;
}

bool is_valid_ipv4_address(std::string_view address) noexcept {
  if (address.empty() || address.size() > 15) {
    return false;
  }
  char buf[16]{};
  for (std::size_t i = 0; i < address.size(); ++i) {
    buf[i] = address[i];
  }
  buf[address.size()] = '\0';

  in_addr sin_addr{};
  return ::inet_pton(AF_INET, buf, &sin_addr) == 1;
}

std::optional<ServerConfig> parse_cli_args(int argc,
                                           char* const argv[]) noexcept {
  if (argc != 5 || argv == nullptr) {
    return std::nullopt;
  }

  for (int i = 0; i < argc; ++i) {
    if (argv[i] == nullptr) {
      return std::nullopt;
    }
  }

  const std::string_view address_str = argv[1];
  if (!is_valid_ipv4_address(address_str)) {
    return std::nullopt;
  }

  ServerConfig config;
  config.address = std::string(address_str);

  if (!parse_port(argv[2], config.port)) {
    return std::nullopt;
  }

  if (!parse_positive_uint64(argv[3], config.n_messages)) {
    return std::nullopt;
  }

  if (!parse_positive_uint64(argv[4], config.timeout_sec)) {
    return std::nullopt;
  }

  return config;
}

void print_usage(std::string_view program_name) noexcept {
  std::cout << "Usage:\n"
            << "  " << program_name
            << " <address> <port> <n_messages> <timeout_sec>\n"
            << "Example:\n"
            << "  " << program_name << " 127.0.0.1 8080 10 5\n";
}

}  // namespace stats
