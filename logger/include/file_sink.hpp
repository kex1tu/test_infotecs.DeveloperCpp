// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <fstream>
#include <string>
#include <string_view>

#include "isink.hpp"
#include "logger_error.hpp"

namespace logger {

class FileSink final : public ISink {
 public:
  FileSink() = default;
  ~FileSink() override;

  LoggerError open(const std::string& filename) noexcept;

  LoggerError write(std::string_view formatted_message) noexcept override;
  void close() noexcept override;
  bool is_open() const noexcept override;

 private:
  std::ofstream file_;
};

}  // namespace logger
