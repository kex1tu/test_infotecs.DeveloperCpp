#include "file_sink.hpp"

namespace logger {

FileSink::~FileSink() { close(); }

LoggerError FileSink::open(const std::string& filename) noexcept {
  if (file_.is_open()) {
    file_.close();
  }
  file_.clear();
  if (filename.empty()) {
    return LoggerError::kInvalidArgument;
  }

  file_.open(filename, std::ios::out | std::ios::app);
  if (!file_.is_open()) {
    return LoggerError::kSinkError;
  }

  return LoggerError::kSuccess;
}

LoggerError FileSink::write(std::string_view formatted_message) noexcept {
  if (!file_.is_open()) {
    return LoggerError::kSinkError;
  }

  file_ << formatted_message;

  if (file_.fail()) {
    return LoggerError::kSinkError;
  }

  return LoggerError::kSuccess;
}

void FileSink::close() noexcept {
  if (file_.is_open()) {
    file_.close();
  }
}

bool FileSink::is_open() const noexcept { return file_.is_open(); }

}  // namespace logger
