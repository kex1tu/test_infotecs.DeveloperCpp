// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.
#include "utilities.hpp"

#include <charconv>
#include <iostream>

namespace app {
// CLI argument parsing implementation
// ////////////////////////////////////

// TODO: Добавить поддержку коротких флагов (-f, -s).
std::optional<AppConfig> parse_cli_args(int argc, char** argv) noexcept {
  if (argc < 2) {
    return std::nullopt;
  }
  AppConfig config;
  const std::string_view arg1 = argv[1];

  if (arg1 == "--file") {
    if (argc < 3) {
      return std::nullopt;
    }
    config.mode = OutputMode::kFile;
    config.filepath = argv[2];
    if (argc >= 4 && !logger::parse_log_level(argv[3], config.min_level)) {
      return std::nullopt;
    }
    return config;
  }

  if (arg1 == "--socket") {
    if (argc < 4) {
      return std::nullopt;
    }
    config.mode = OutputMode::kSocket;
    config.remote_addr = argv[2];
    if (!parse_port(argv[3], config.port)) {
      return std::nullopt;
    }
    if (argc >= 5 && !logger::parse_log_level(argv[4], config.min_level)) {
      return std::nullopt;
    }
    return config;
  }

  return std::nullopt;
}

// используем std::from_chars для быстрого парсинга строки в число
bool parse_port(std::string_view str, uint16_t& port) noexcept {
  if (str.empty()) {
    return false;
  }
  int val = 0;
  auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (err == std::errc{} && ptr == str.data() + str.size() && val >= 1 &&
      val <= 65535) {
    port = static_cast<uint16_t>(val);
    return true;
  }
  return false;
}

void print_usage(std::string_view program_name) noexcept {
  std::cout << "Usage:\n"
            << "  " << program_name
            << " --file <log_file> [level: DEBUG|INFO|WARNING|ERROR]\n"
            << "  " << program_name
            << " --socket <address> <port> [level: DEBUG|INFO|WARNING|ERROR]\n";
}
// User input parsing helpers
// ///////////////////////////

// Парсинг префикса уровня лога вида "[LEVEL] message". Если закрывающая скобка
// ']' не найдена или уровень не распознан, вся исходная строка считается телом
// сообщения с уровнем по умолчанию.
LogItem parse_input(std::string_view input,
                    logger::LogLevel default_level) noexcept {
  const size_t start = input.find_first_not_of(" \t");
  if (start == std::string_view::npos) {
    return {default_level, ""};
  }
  input.remove_prefix(start);
  if (input.front() != '[') {
    return {default_level, std::string(input)};
  }
  const size_t close_bracket = input.find(']');
  if (close_bracket == std::string_view::npos) {
    return {default_level, std::string(input)};
  }
  const std::string_view level_str = input.substr(1, close_bracket - 1);
  logger::LogLevel parsed_level{};
  if (!logger::parse_log_level(level_str, parsed_level)) {
    return {default_level, std::string(input)};
  }

  std::string_view msg = input.substr(close_bracket + 1);
  const size_t msg_start = msg.find_first_not_of(" \t");
  if (msg_start != std::string_view::npos) {
    msg.remove_prefix(msg_start);
  } else {
    msg = {};
  }

  return {parsed_level, std::string(msg)};
}

}  // namespace app
