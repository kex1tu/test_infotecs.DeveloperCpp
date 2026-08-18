// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

// NOLINTBEGIN
#include <cctype>
#include <fstream>
#include <string_view>
#include <vector>

#include "file_sink.hpp"
#include "logger.hpp"
#include "logger_error.hpp"
#include "logger_factories.hpp"
#include "tests.hpp"

bool test_1() {
  using namespace logger;

  ASSERT_EQ(std::string(log_level_to_string(LogLevel::kDebug)), "DEBUG");
  ASSERT_EQ(std::string(log_level_to_string(LogLevel::kInfo)), "INFO");
  ASSERT_EQ(std::string(log_level_to_string(LogLevel::kWarning)), "WARNING");
  ASSERT_EQ(std::string(log_level_to_string(LogLevel::kError)), "ERROR");
  ASSERT_EQ(std::string(log_level_to_string(LogLevel::kUnknown)), "UNKNOWN");

  LogLevel parsed{};
  ASSERT_TRUE(parse_log_level("DEBUG", parsed));
  ASSERT_EQ(parsed, LogLevel::kDebug);

  ASSERT_TRUE(parse_log_level("info", parsed));
  ASSERT_EQ(parsed, LogLevel::kInfo);

  ASSERT_TRUE(parse_log_level("WarnIng", parsed));
  ASSERT_EQ(parsed, LogLevel::kWarning);

  ASSERT_TRUE(parse_log_level("ERROR", parsed));
  ASSERT_EQ(parsed, LogLevel::kError);

  ASSERT_TRUE(parse_log_level("Unknown", parsed));
  ASSERT_EQ(parsed, LogLevel::kUnknown);

  ASSERT_TRUE(!parse_log_level("INVALID", parsed));
  ASSERT_TRUE(!parse_log_level("", parsed));
  ASSERT_TRUE(!parse_log_level("d\n\0", parsed));

  return true;
}

bool test_2() {
  logger::Logger log(nullptr);

  ASSERT_EQ(log.log_message(logger::LogLevel::kInfo, "test"),
            logger::LoggerError::kNotInitialized);

  return true;
}

bool test_3() {
  using namespace logger;

  auto [err_empty, log_empty] = make_file_logger("");
  ASSERT_EQ(err_empty, LoggerError::kInvalidArgument);
  ASSERT_TRUE(log_empty == nullptr);

  auto [err_file, log_file] =
      make_file_logger("test_init.log", LogLevel::kDebug);
  ASSERT_EQ(err_file, LoggerError::kSuccess);
  ASSERT_TRUE(log_file != nullptr);
  log_file->close();
  std::remove("test_init.log");

  auto [err_null, log_null] = make_custom_logger(nullptr);
  ASSERT_EQ(err_null, LoggerError::kInvalidArgument);
  ASSERT_TRUE(log_null == nullptr);

  auto sink = std::make_unique<MemorySink>();
  auto [err_custom, log_custom] =
      make_custom_logger(std::move(sink), LogLevel::kInfo);
  ASSERT_EQ(err_custom, LoggerError::kSuccess);
  ASSERT_TRUE(log_custom != nullptr);

  return true;
}

bool test_4() {
  using namespace logger;

  auto sink = std::make_unique<MemorySink>();
  MemorySink* raw_sink = sink.get();

  auto [err, log] = make_custom_logger(std::move(sink), LogLevel::kWarning);
  ASSERT_EQ(err, LoggerError::kSuccess);
  ASSERT_TRUE(log != nullptr);

  ASSERT_EQ(log->log_message(LogLevel::kDebug, "debug message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log->log_message(LogLevel::kInfo, "info message"),
            LoggerError::kSuccess);
  ASSERT_EQ(raw_sink->messages.size(), 0);
  ASSERT_EQ(log->log_message(LogLevel::kWarning, "warning message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log->log_message(LogLevel::kError, "error message"),
            LoggerError::kSuccess);
  ASSERT_EQ(raw_sink->messages.size(), 2);
  ASSERT_TRUE(raw_sink->messages[0].find("[WARNING] warning message") !=
              std::string::npos);
  ASSERT_TRUE(raw_sink->messages[1].find("[ERROR] error message") !=
              std::string::npos);

  raw_sink->messages.clear();
  ASSERT_EQ(log->set_level(LogLevel::kError), LoggerError::kSuccess);
  ASSERT_EQ(log->get_level(), LogLevel::kError);
  ASSERT_EQ(log->log_message(LogLevel::kDebug, "debug message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log->log_message(LogLevel::kInfo, "info message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log->log_message(LogLevel::kWarning, "warning message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log->log_message(LogLevel::kError, "error message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log->log_message(LogLevel::kUnknown, "unknown message"),
            LoggerError::kSuccess);
  ASSERT_EQ(raw_sink->messages.size(), 2);
  ASSERT_TRUE(raw_sink->messages[0].find("[ERROR] error message") !=
              std::string::npos);
  ASSERT_TRUE(raw_sink->messages[1].find("[UNKNOWN] unknown message") !=
              std::string::npos);
  return true;
}

bool test_5() {
  using namespace logger;

  auto sink = std::make_unique<MemorySink>();
  ASSERT_TRUE(sink->is_open());

  auto [err, log] = make_custom_logger(std::move(sink), LogLevel::kDebug);
  ASSERT_EQ(err, LoggerError::kSuccess);
  ASSERT_TRUE(log != nullptr);

  log->close();

  ASSERT_EQ(log->log_message(LogLevel::kInfo, "msg after close"),
            LoggerError::kNotInitialized);

  auto new_sink = std::make_unique<MemorySink>();
  ASSERT_TRUE(new_sink->messages.empty());
  auto [err_new, new_log] =
      make_custom_logger(std::move(new_sink), LogLevel::kDebug);
  ASSERT_EQ(err_new, LoggerError::kSuccess);
  ASSERT_TRUE(new_log != nullptr);

  return true;
}

bool test_6() {
  using namespace logger;

  const std::string filename = "test_output.log";
  std::remove(filename.c_str());

  auto [err, log] = make_file_logger(filename, LogLevel::kInfo);
  ASSERT_EQ(err, LoggerError::kSuccess);

  ASSERT_EQ(log->log_message(LogLevel::kInfo, "file logging works"),
            LoggerError::kSuccess);
  log->close();

  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open());

  std::string line;
  std::getline(file, line);
  file.close();

  ASSERT_TRUE(line.find("[INFO] file logging works") != std::string::npos);

  std::remove(filename.c_str());

  return true;
}

bool test_7() {
  using namespace logger;

  auto sink = std::make_unique<MemorySink>();
  MemorySink* raw_sink = sink.get();

  auto [err, log] = make_custom_logger(std::move(sink), LogLevel::kDebug);
  ASSERT_EQ(err, LoggerError::kSuccess);
  ASSERT_TRUE(log != nullptr);

  ASSERT_EQ(log->log_message(LogLevel::kInfo, "test format message"),
            LoggerError::kSuccess);
  ASSERT_EQ(raw_sink->messages.size(), 1);

  const std::string& msg = raw_sink->messages[0];
  ASSERT_TRUE(msg.size() >= 25);
  ASSERT_EQ(msg[0], '[');
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[1])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[2])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[3])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[4])));
  ASSERT_EQ(msg[5], '-');
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[6])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[7])));
  ASSERT_EQ(msg[8], '-');
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[9])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[10])));
  ASSERT_EQ(msg[11], ' ');
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[12])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[13])));
  ASSERT_EQ(msg[14], ':');
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[15])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[16])));
  ASSERT_EQ(msg[17], ':');
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[18])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[19])));
  ASSERT_EQ(msg[20], '.');
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[21])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[22])));
  ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(msg[23])));
  ASSERT_EQ(msg[24], ']');
  ASSERT_EQ(msg.back(), '\n');

  const std::string expected_suffix = " [INFO] test format message\n";
  ASSERT_TRUE(msg.size() > expected_suffix.size());
  ASSERT_EQ(msg.substr(msg.size() - expected_suffix.size()), expected_suffix);

  return true;
}

bool test_8() {
  using namespace logger;

  auto sink = std::make_unique<MemorySink>();
  MemorySink* raw_sink = sink.get();

  auto [err, log] = make_custom_logger(std::move(sink), LogLevel::kDebug);
  ASSERT_EQ(err, LoggerError::kSuccess);
  ASSERT_TRUE(log != nullptr);

  ASSERT_EQ(log->log_message(LogLevel::kInfo, ""),
            LoggerError::kInvalidArgument);
  ASSERT_EQ(log->log_message(LogLevel::kError, std::string_view{}),
            LoggerError::kInvalidArgument);
  ASSERT_EQ(raw_sink->messages.size(), 0);

  return true;
}

bool test_9() {
  using namespace logger;

  FileSink sink;
  ASSERT_TRUE(!sink.is_open());
  ASSERT_EQ(sink.write("closed sink write"), LoggerError::kSinkError);

  const std::string temp_file = "test_sink.log";
  std::remove(temp_file.c_str());
  ASSERT_EQ(sink.open(temp_file), LoggerError::kSuccess);
  ASSERT_TRUE(sink.is_open());
  sink.close();
  ASSERT_TRUE(!sink.is_open());
  ASSERT_EQ(sink.write("write after close"), LoggerError::kSinkError);
  std::remove(temp_file.c_str());

  ASSERT_EQ(sink.open("/non_exist_dir/test.log"), LoggerError::kSinkError);
  ASSERT_TRUE(!sink.is_open());

  auto [err_invalid, log_invalid] = make_file_logger("/non_exist_dir/test.log");
  ASSERT_EQ(err_invalid, LoggerError::kSinkError);
  ASSERT_TRUE(log_invalid == nullptr);

  return true;
}

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Тесты LOGGER: \n";
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

  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}
// NOLINTEND