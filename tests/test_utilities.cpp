// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include <string>
#include <vector>

#include "tests.hpp"
#include "utilities.hpp"

namespace {

// вспомогательная функция для тестов
std::optional<app::AppConfig> run_parse_cli(
    const std::vector<std::string>& args) noexcept {
  std::vector<char*> argv;
  argv.reserve(args.size());
  for (const auto& arg : args) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  return app::parse_cli_args(static_cast<int>(argv.size()), argv.data());
}

}  // namespace

// тесты parse_port корректных портов
bool test_1() {
  uint16_t port = 0;

  ASSERT_TRUE(app::parse_port("1", port));
  ASSERT_EQ(port, 1);

  ASSERT_TRUE(app::parse_port("80", port));
  ASSERT_EQ(port, 80);

  ASSERT_TRUE(app::parse_port("8080", port));
  ASSERT_EQ(port, 8080);

  ASSERT_TRUE(app::parse_port("65535", port));
  ASSERT_EQ(port, 65535);

  return true;
}

// тесты parse_port невалидных портов
bool test_2() {
  uint16_t port = 0;

  ASSERT_TRUE(!app::parse_port("0", port));
  ASSERT_TRUE(!app::parse_port("65536", port));
  ASSERT_TRUE(!app::parse_port("-1", port));
  ASSERT_TRUE(!app::parse_port("", port));

  return true;
}

// тесты parse_port мусора
bool test_3() {
  uint16_t port = 0;

  ASSERT_TRUE(!app::parse_port("abc", port));
  ASSERT_TRUE(!app::parse_port("8080abc", port));
  ASSERT_TRUE(!app::parse_port(" 80", port));

  return true;
}

// тесты parse_cli_args --file
bool test_4() {
  // дефолтный уровнь debug
  auto res_default = run_parse_cli({"./app", "--file", "app.log"});
  ASSERT_TRUE(res_default.has_value());
  ASSERT_TRUE(res_default->mode == app::OutputMode::kFile);
  ASSERT_EQ(res_default->filepath, "app.log");
  ASSERT_EQ(res_default->min_level, logger::LogLevel::kDebug);

  // явно указан INFO
  auto res_info = run_parse_cli({"./app", "--file", "app.log", "INFO"});
  ASSERT_TRUE(res_info.has_value());
  ASSERT_TRUE(res_info->mode == app::OutputMode::kFile);
  ASSERT_EQ(res_info->filepath, "app.log");
  ASSERT_EQ(res_info->min_level, logger::LogLevel::kInfo);

  // явно указа ERROR
  auto res_error = run_parse_cli({"./app", "--file", "app.log", "ERROR"});
  ASSERT_TRUE(res_error.has_value());
  ASSERT_TRUE(res_error->mode == app::OutputMode::kFile);
  ASSERT_EQ(res_error->filepath, "app.log");
  ASSERT_EQ(res_error->min_level, logger::LogLevel::kError);

  return true;
}

// тесты parse_cli_args --socket
bool test_5() {
  // дефолтный уровень DEBUG
  auto res_default = run_parse_cli({"./app", "--socket", "127.0.0.1", "8080"});
  ASSERT_TRUE(res_default.has_value());
  ASSERT_TRUE(res_default->mode == app::OutputMode::kSocket);
  ASSERT_EQ(res_default->remote_addr, "127.0.0.1");
  ASSERT_EQ(res_default->port, 8080);
  ASSERT_EQ(res_default->min_level, logger::LogLevel::kDebug);

  // явный валидный порт и уровень
  auto res_warning =
      run_parse_cli({"./app", "--socket", "localhost", "9000", "WARNING"});
  ASSERT_TRUE(res_warning.has_value());
  ASSERT_TRUE(res_warning->mode == app::OutputMode::kSocket);
  ASSERT_EQ(res_warning->remote_addr, "localhost");
  ASSERT_EQ(res_warning->port, 9000);
  ASSERT_EQ(res_warning->min_level, logger::LogLevel::kWarning);

  return true;
}

// мало аргументов
bool test_6() {
  auto res_file = run_parse_cli({"./app", "--file"});
  ASSERT_TRUE(!res_file.has_value());

  auto res_socket_empty = run_parse_cli({"./app", "--socket"});
  ASSERT_TRUE(!res_socket_empty.has_value());

  auto res_socket_no_port = run_parse_cli({"./app", "--socket", "127.0.0.1"});
  ASSERT_TRUE(!res_socket_no_port.has_value());

  return true;
}

// неизвестные флаги и пустой флаг
bool test_7() {
  auto res_udp = run_parse_cli({"./app", "--unknowsn", "127.0.0.1", "8080"});
  ASSERT_TRUE(!res_udp.has_value());

  auto res_short = run_parse_cli({"./app", "-f", "app.log"});
  ASSERT_TRUE(!res_short.has_value());

  auto res_empty = run_parse_cli({"./app"});
  ASSERT_TRUE(!res_empty.has_value());

  return true;
}

// невалидные значения уровня лога
bool test_8() {
  auto res_file =
      run_parse_cli({"./app", "--file", "test.log", "INVALID_LEVEL"});
  ASSERT_TRUE(!res_file.has_value());

  auto res_socket = run_parse_cli(
      {"./app", "--socket", "127.0.0.1", "8080", "INVALID_LEVEL"});
  ASSERT_TRUE(!res_socket.has_value());

  return true;
}

// тест parse_input строка с уровнем
bool test_9() {
  auto item1 = app::parse_input("[INFO] User logged in");
  ASSERT_EQ(item1.level, logger::LogLevel::kInfo);
  ASSERT_EQ(item1.message, "User logged in");

  auto item2 = app::parse_input("[ERROR] Connection failed");
  ASSERT_EQ(item2.level, logger::LogLevel::kError);
  ASSERT_EQ(item2.message, "Connection failed");

  return true;
}

// Тесты parse_input c разным регистром
bool test_10() {
  auto item1 = app::parse_input("[dEbUG] msg");
  ASSERT_EQ(item1.level, logger::LogLevel::kDebug);
  ASSERT_EQ(item1.message, "msg");

  auto item2 = app::parse_input("[wArNiNg] msg");
  ASSERT_EQ(item2.level, logger::LogLevel::kWarning);
  ASSERT_EQ(item2.message, "msg");

  return true;
}

// parse_input лишние пробелы
bool test_11() {
  auto item = app::parse_input("   [INFO]   hello world");
  ASSERT_EQ(item.level, logger::LogLevel::kInfo);
  ASSERT_EQ(item.message, "hello world");

  return true;
}

// строка без префикса уровня
bool test_12() {
  auto item1 = app::parse_input("Regular text message");
  ASSERT_EQ(item1.level, logger::LogLevel::kDebug);
  ASSERT_EQ(item1.message, "Regular text message");
  auto item2 =
      app::parse_input("Regular text message", logger::LogLevel::kWarning);
  ASSERT_EQ(item2.level, logger::LogLevel::kWarning);
  ASSERT_EQ(item2.message, "Regular text message");

  return true;
}

// parse_input строка с неизвестным префиксом
bool test_13() {
  auto item = app::parse_input("[CUSTOM] text");
  ASSERT_EQ(item.level, logger::LogLevel::kDebug);
  ASSERT_EQ(item.message, "[CUSTOM] text");

  return true;
}

// parse_input пустая строка и строка только из пробелов
bool test_14() {
  auto item_empty = app::parse_input("");
  ASSERT_TRUE(item_empty.message.empty());

  auto item_spaces = app::parse_input("   \t  ");
  ASSERT_TRUE(item_spaces.message.empty());

  return true;
}

// parse_input непарные скобки
bool test_15() {
  auto item1 = app::parse_input("[INFO message");
  ASSERT_EQ(item1.level, logger::LogLevel::kDebug);
  ASSERT_EQ(item1.message, "[INFO message");

  auto item2 = app::parse_input("INFO] message");
  ASSERT_EQ(item2.level, logger::LogLevel::kDebug);
  ASSERT_EQ(item2.message, "INFO] message");

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
  RUN_TEST(test_7);
  RUN_TEST(test_8);
  RUN_TEST(test_9);
  RUN_TEST(test_10);
  RUN_TEST(test_11);
  RUN_TEST(test_12);
  RUN_TEST(test_13);
  RUN_TEST(test_14);
  RUN_TEST(test_15);

  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}
