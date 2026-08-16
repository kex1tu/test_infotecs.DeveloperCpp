#pragma once

#include <string_view>

#include "logger_error.hpp"

namespace logger {
// интерфейс с которым взаимодействует логгер для записи данных
class ISink {
 public:
  virtual ~ISink() = default;

  virtual LoggerError write(std::string_view formatted_message) noexcept = 0;
  virtual void close() noexcept = 0;
  virtual bool is_open() const noexcept = 0;

  ISink(const ISink&) = delete;
  ISink& operator=(const ISink&) = delete;
  ISink(ISink&&) = delete;
  ISink& operator=(ISink&&) = delete;

 protected:
  ISink() = default;
};

}  // namespace logger
