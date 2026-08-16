#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "log_level.hpp"
#include "logger.hpp"
#include "logger_error.hpp"
#include "sink.hpp"

// макросы проверки условий
#define ASSERT_TRUE(cond)                                                   \
  do {                                                                      \
    if (!(cond)) {                                                          \
      std::cerr << "  [FAIL] " << #cond << " at line " << __LINE__ << '\n'; \
      return false;                                                         \
    }                                                                       \
  } while (0)

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

// макрос запуска теста
#define RUN_TEST(test_func)                           \
  do {                                                \
    std::cout << "[RUN ] " << #test_func;             \
    if (test_func()) {                                \
      std::cout << "\n[  OK] " << #test_func << '\n'; \
      ++passed;                                       \
    }                                                 \
    ++total;                                          \
  } while (0)

// sink для тестов
class MemorySink final : public logger::ISink {
 public:
  bool is_open_{true};
  std::vector<std::string> messages;

  logger::LoggerError write(
      std::string_view formatted_message) noexcept override {
    if (!is_open_) {
      return logger::LoggerError::kWriteFailed;
    }
    messages.emplace_back(formatted_message);
    return logger::LoggerError::kSuccess;
  }

  void close() noexcept override { is_open_ = false; }

  bool is_open() const noexcept override { return is_open_; }
};
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
  logger::Logger log;

  ASSERT_EQ(log.log_message(logger::LogLevel::kInfo, "test"),
            logger::LoggerError::kNotInitialized);
  ASSERT_EQ(log.set_level(logger::LogLevel::kError),
            logger::LoggerError::kNotInitialized);

  return true;
}
// проверка инициализации
bool test_3() {
  using namespace logger;

  Logger log;

  ASSERT_EQ(log.init_with_file(""), LoggerError::kInvalidArgument);
  ASSERT_EQ(log.init_with_file("tmp/1.log", LogLevel::kDebug),
            LoggerError::kSuccess);
  ASSERT_EQ(log.init_with_file("tmp/1.log", LogLevel::kInfo),
            LoggerError::kAlreadyInitialized);
  log.close();

  ASSERT_EQ(log.init_with_any_sink(nullptr), LoggerError::kInvalidArgument);

  auto sink = std::make_unique<MemorySink>();
  ASSERT_EQ(log.init_with_any_sink(std::move(sink)), LoggerError::kSuccess);

  return true;
}

// тест на работу с фильтром по уровню
bool test_4() {
  using namespace logger;

  Logger log;
  auto sink = std::make_unique<MemorySink>();
  MemorySink* raw_sink = sink.get();
  log.init_with_any_sink(std::move(sink), LogLevel::kWarning);
  ASSERT_EQ(log.log_message(LogLevel::kDebug, "debug message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log.log_message(LogLevel::kInfo, "info message"),
            LoggerError::kSuccess);
  ASSERT_EQ(raw_sink->messages.size(), 0);
  ASSERT_EQ(log.log_message(LogLevel::kWarning, "warning message"),
            LoggerError::kSuccess);
  ASSERT_EQ(log.log_message(LogLevel::kError, "error message"),
            LoggerError::kSuccess);
  ASSERT_EQ(raw_sink->messages.size(), 2);
  ASSERT_TRUE(raw_sink->messages[0].find("[WARNING] warning message") !=
              std::string::npos);
  ASSERT_TRUE(raw_sink->messages[1].find("[ERROR] error message") !=
              std::string::npos);

  raw_sink->messages.clear();
  log.set_level(LogLevel::kError);
  log.log_message(LogLevel::kDebug, "debug message");
  log.log_message(LogLevel::kInfo, "info message");
  log.log_message(LogLevel::kWarning, "warning message");
  log.log_message(LogLevel::kError, "error message");
  log.log_message(LogLevel::kUnknown, "unknown message");
  ASSERT_EQ(raw_sink->messages.size(), 2);
  ASSERT_TRUE(raw_sink->messages[0].find("[ERROR] error message") !=
              std::string::npos);
  ASSERT_TRUE(raw_sink->messages[1].find("[UNKNOWN] unknown message") !=
              std::string::npos);
  return true;
}

// тест на закрытие и открытие с новым синком
bool test_5() {
  using namespace logger;

  Logger log;
  auto sink = std::make_unique<MemorySink>();
  MemorySink* raw_sink = sink.get();

  log.init_with_any_sink(std::move(sink), LogLevel::kDebug);
  ASSERT_TRUE(raw_sink->is_open());

  log.close();

  ASSERT_EQ(log.log_message(LogLevel::kInfo, "msg after close"),
            LoggerError::kNotInitialized);

  auto new_sink = std::make_unique<MemorySink>();
  MemorySink* raw_new_sink = new_sink.get();
  ASSERT_EQ(log.init_with_any_sink(std::move(new_sink), LogLevel::kDebug),
            LoggerError::kSuccess);
  ASSERT_EQ(log.set_level(LogLevel::kDebug), LoggerError::kSuccess);
  ASSERT_TRUE(raw_new_sink->messages.empty());

  return true;
}
// запись в реальный файл
bool test_6() {
  using namespace logger;

  const std::string filename = "test_output.log";
  std::remove(filename.c_str());

  Logger log;
  ASSERT_EQ(log.init_with_file(filename, LogLevel::kInfo),
            LoggerError::kSuccess);

  ASSERT_EQ(log.log_message(LogLevel::kInfo, "file logging works"),
            LoggerError::kSuccess);
  log.close();

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