// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "log_level.hpp"
#include "logger.hpp"
#include "logger_error.hpp"
#include "logger_factories.hpp"
#include "thread_safe_queue.hpp"
#include "utilities.hpp"

int main(int argc, char** argv) {
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  auto config = app::parse_cli_args(argc, argv);
  if (!config) {
    app::print_usage(argv[0]);
    return 1;
  }

  auto loggre_res =
      (config->mode == app::OutputMode::kFile)
          ? logger::make_file_logger(config->filepath, config->min_level)
          : logger::make_socket_logger(config->remote_addr, config->port,
                                       config->min_level);
  auto err = loggre_res.first;
  auto log_ptr = std::move(loggre_res.second);
  if (err != logger::LoggerError::kSuccess) {
    std::cerr << "Error creating logger: "
              << logger::logger_error_to_string(err) << '\n';
    return 1;
  }
  std::cout << "LOGGER CREATED SUCCESSFULLY\n";
  std::cout << "CONFIG:\n";
  std::cout << "MODE: "
            << ((config->mode == app::OutputMode::kFile) ? "FILE" : "SOCKET")
            << '\n';
  std::cout << "FILEPATH: " << config->filepath << '\n';
  std::cout << "REMOTE_ADDR: " << config->remote_addr << '\n';
  std::cout << "PORT: " << config->port << '\n';
  std::cout << "MIN_LEVEL: " << logger::log_level_to_string(config->min_level)
            << '\n';
  std::cout << std::flush;

  logger::ThreadSafeQueue<app::LogItem> queue;
  std::thread logger_thread([&]() {
    while (auto log_pair = queue.wait_and_pop()) {
      auto err = log_ptr->log_message(log_pair->level, log_pair->message);
      if (err != logger::LoggerError::kSuccess) {
        std::cerr << "Error writing log message: "
                  << logger::logger_error_to_string(err) << '\n';
      }
    }
    log_ptr->close();
  });

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line == "exit") {
      break;
    }
    if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
      continue;
    }
    app::LogItem log_item = app::parse_input(line, config->min_level);

    if (!queue.push(std::move(log_item))) {
      std::cerr << "Error pushing log message to queue" << '\n';
    }
  }

  queue.stop();
  if (logger_thread.joinable()) {
    logger_thread.join();
  }
  return 0;
}