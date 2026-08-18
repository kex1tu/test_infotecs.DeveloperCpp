// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

// NOLINTBEGIN

#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "isink.hpp"
#include "logger_error.hpp"

#define ASSERT_TRUE(cond)                                                   \
  do {                                                                      \
    if (!(cond)) {                                                          \
      std::cerr << "  [FAIL] " << #cond << " at line " << __LINE__ << '\n'; \
      return false;                                                         \
    }                                                                       \
  } while (0)

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

#define RUN_TEST(test_func)                           \
  do {                                                \
    std::cout << "[RUN ] " << #test_func;             \
    if (test_func()) {                                \
      std::cout << "\n[  OK] " << #test_func << '\n'; \
      ++passed;                                       \
    }                                                 \
    ++total;                                          \
  } while (0)

class MemorySink final : public logger::ISink {
 public:
  bool is_open_{true};
  std::vector<std::string> messages;

  logger::LoggerError write(
      std::string_view formatted_message) noexcept override {
    try {
      if (!is_open_) {
        return logger::LoggerError::kSinkError;
      }
      messages.emplace_back(formatted_message);
      return logger::LoggerError::kSuccess;
    } catch (...) {
      return logger::LoggerError::kSinkError;
    }
  }

  void close() noexcept override { is_open_ = false; }

  bool is_open() const noexcept override { return is_open_; }
};
// NOLINTEND