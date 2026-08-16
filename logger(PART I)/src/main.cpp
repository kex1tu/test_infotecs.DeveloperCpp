#include "../include/logger.hpp"

int main() {
  logger::Logger log;
  log.init_with_file("1.txt", logger::LogLevel::kDebug);
  log.log_message(logger::LogLevel::kDebug, "debug");
  log.log_message(logger::LogLevel::kInfo, "info");
  log.log_message(logger::LogLevel::kWarning, "warning");
  log.log_message(logger::LogLevel::kError, "error");
  log.close();

  log.init_with_file("1.txt", logger::LogLevel::kWarning);
  log.log_message(logger::LogLevel::kDebug, "debug");
  log.log_message(logger::LogLevel::kInfo, "info");
  log.log_message(logger::LogLevel::kWarning, "warning");
  log.log_message(logger::LogLevel::kError, "error");
  log.close();
  log.set_level(logger::LogLevel::kInfo);

  log.init_with_socket("[IP_ADDRESS]", 12345, logger::LogLevel::kDebug);
  log.log_message(logger::LogLevel::kDebug, "debug");
  log.log_message(logger::LogLevel::kInfo, "info");
  log.log_message(logger::LogLevel::kWarning, "warning");
  log.log_message(logger::LogLevel::kError, "error");
  log.close();

  return 0;
}