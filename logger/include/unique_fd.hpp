// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <unistd.h>

namespace logger {

class UniqueFd {
 public:
  constexpr UniqueFd() noexcept = default;
  explicit UniqueFd(int fd) noexcept : fd_(fd) {}
  ~UniqueFd() noexcept { reset(); }

  UniqueFd(const UniqueFd&) = delete;
  UniqueFd& operator=(const UniqueFd&) = delete;

  UniqueFd(UniqueFd&& other) noexcept : fd_(other.release()) {}
  UniqueFd& operator=(UniqueFd&& other) noexcept {
    if (this != &other) {
      reset(other.release());
    }
    return *this;
  }

  [[nodiscard]] int get() const noexcept { return fd_; }
  [[nodiscard]] bool is_valid() const noexcept { return fd_ >= 0; }

  int release() noexcept {
    const int old_fd = fd_;
    fd_ = -1;
    return old_fd;
  }

  void reset(int new_fd = -1) noexcept {
    if (fd_ >= 0) {
      ::close(fd_);
    }
    fd_ = new_fd;
  }

 private:
  int fd_ = -1;
};

}  // namespace logger
