#pragma once

#include <fstream>
#include <string>
#include <string_view>

#include "isink.hpp"
#include "logger_error.hpp"

namespace logger {
// для работы с файлом
class FileSink final : public ISink {
 public:
  FileSink() = default;
  ~FileSink() override;

  FileSink(const FileSink&) = delete;
  FileSink& operator=(const FileSink&) = delete;
  FileSink(FileSink&&) = delete;
  FileSink& operator=(FileSink&&) = delete;

  LoggerError open(const std::string& filename) noexcept;
  LoggerError write(std::string_view formatted_message) noexcept override;
  void close() noexcept override;
  bool is_open() const noexcept override;

 private:
  std::ofstream file_;
};

}  // namespace logger
