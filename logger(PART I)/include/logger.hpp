#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "log_level.hpp"
#include "logger_error.hpp"
#include "isink.hpp"

namespace logger {
// логгер, можно использовать с файлом, сокетом или с чем-то что реализует
// интерфейс ISink
// конструкторы и семантика перемещения запрещены
// все методы отмечены как noexcept и таковыми являются
// close() используется для закрытия логгера, после закрытия логгер не может
// быть использован
// каждая функция возвращает LoggerError:
// kSuccess - в случае успеха
// kNotInitialized - если логгер не инициализирован
// kAlreadyInitialized - если логгер уже инициализирован
// kInvalidArgument - если аргументы функции некорректны
// kSinkError - если возникла ошибка при работе с sink

class Logger {
 public:
  explicit Logger(std::unique_ptr<ISink> sink,
                  LogLevel min_level = LogLevel::kDebug) noexcept
      : sink_(std::move(sink)), min_level_(min_level) {}
  ~Logger() noexcept { close(); }

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  LoggerError log_message(LogLevel level, std::string_view message) noexcept;

  LoggerError set_level(LogLevel level) noexcept;

  LogLevel get_level() const noexcept;

  void close() noexcept;

 private:
  static std::string format_message(LogLevel level,
                                    std::string_view message) noexcept;

  std::unique_ptr<ISink> sink_;
  LogLevel min_level_{LogLevel::kDebug};
};

}  // namespace logger
