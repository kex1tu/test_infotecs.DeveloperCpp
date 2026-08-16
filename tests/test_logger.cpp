#include <fstream>
#include <string_view>
#include <vector>

#include "logger.hpp"
#include "logger_error.hpp"
#include "logger_factories.hpp"
#include "tests.hpp"

// из уровня лога строку и обратно
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

// запуск без инициализации
bool test_2() {
  logger::Logger log(nullptr);

  ASSERT_EQ(log.log_message(logger::LogLevel::kInfo, "test"),
            logger::LoggerError::kNotInitialized);

  return true;
}

// проверка фабрик
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

// тест на работу с фильтром по уровню
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

// тест на закрытие и создание нового логгера
bool test_5() {
  using namespace logger;

  auto sink = std::make_unique<MemorySink>();
  MemorySink* raw_sink = sink.get();

  auto [err, log] = make_custom_logger(std::move(sink), LogLevel::kDebug);
  ASSERT_EQ(err, LoggerError::kSuccess);
  ASSERT_TRUE(raw_sink->is_open());

  log->close();

  ASSERT_EQ(log->log_message(LogLevel::kInfo, "msg after close"),
            LoggerError::kNotInitialized);

  auto new_sink = std::make_unique<MemorySink>();
  MemorySink* raw_new_sink = new_sink.get();
  auto [err_new, new_log] =
      make_custom_logger(std::move(new_sink), LogLevel::kDebug);
  ASSERT_EQ(err_new, LoggerError::kSuccess);
  ASSERT_TRUE(raw_new_sink->messages.empty());

  return true;
}

// запись в реальный файл
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

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Тесты: \n";
  std::cout << "==========================================\n";
  RUN_TEST(test_1);
  RUN_TEST(test_2);
  RUN_TEST(test_3);
  RUN_TEST(test_4);
  RUN_TEST(test_5);
  RUN_TEST(test_6);

  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}