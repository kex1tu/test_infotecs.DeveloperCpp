// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "file_sink.hpp"
#include "log_level.hpp"
#include "logger.hpp"
#include "logger_error.hpp"
#include "socket_sink.hpp"

namespace logger {

/**
 * \brief Создает и инициализирует логгер для записи в файл.
 *
 * Examples:
 * \code
 * auto [err, log] = logger::make_file_logger("app.log", LogLevel::kInfo);
 * if (err == LoggerError::kSuccess) {
 *   log->log_message(LogLevel::kInfo, "App started");
 * }
 * \endcode
 *
 * \param[in] filename Путь к файлу лога.
 * \param[in] min_level Минимальный уровень логирования.
 * \return Пара, содержащая код ошибки и уникальный указатель на Logger.
 */
inline std::pair<LoggerError, std::unique_ptr<Logger>> make_file_logger(
    const std::string& filename,
    LogLevel min_level = LogLevel::kDebug) noexcept {
  try {
    auto sink = std::make_unique<FileSink>();
    const LoggerError err = sink->open(filename);
    if (err != LoggerError::kSuccess) {
      return {err, nullptr};
    }

    auto log = std::make_unique<Logger>(std::move(sink), min_level);
    return {LoggerError::kSuccess, std::move(log)};
  } catch (...) {
    return {LoggerError::kSinkError, nullptr};
  }
}

/**
 * \brief Создает и инициализирует логгер для передачи записей по TCP-сокету.
 *
 * Examples:
 * \code
 * auto [err, log] = logger::make_socket_logger("127.0.0.1", 5140,
 * LogLevel::kDebug); if (err == LoggerError::kSuccess) {
 *   log->log_message(LogLevel::kDebug, "Connected");
 * }
 * \endcode
 *
 * \param[in] address IPv4-адрес сервера.
 * \param[in] port Номер TCP-порта [1..65535].
 * \param[in] min_level Минимальный уровень логирования.
 * \return Пара, содержащая код ошибки и уникальный указатель на Logger.
 */
inline std::pair<LoggerError, std::unique_ptr<Logger>> make_socket_logger(
    const std::string& address, int port,
    LogLevel min_level = LogLevel::kDebug) noexcept {
  try {
    auto sink = std::make_unique<SocketSink>();
    const LoggerError err = sink->connect(address, port);
    if (err != LoggerError::kSuccess) {
      return {err, nullptr};
    }

    auto log = std::make_unique<Logger>(std::move(sink), min_level);
    return {LoggerError::kSuccess, std::move(log)};
  } catch (...) {
    return {LoggerError::kSinkError, nullptr};
  }
}

/**
 * \brief Создает логгер с произвольной пользовательской реализацией ISink.
 *
 * \param[in] sink Уникальный указатель на предварительно открытый ISink.
 * \param[in] min_level Минимальный уровень логирования.
 * \return Пара, содержащая код ошибки и уникальный указатель на Logger.
 */
inline std::pair<LoggerError, std::unique_ptr<Logger>> make_custom_logger(
    std::unique_ptr<ISink> sink,
    LogLevel min_level = LogLevel::kDebug) noexcept {
  try {
    if (!sink || !sink->is_open()) {
      return {LoggerError::kInvalidArgument, nullptr};
    }

    auto log = std::make_unique<Logger>(std::move(sink), min_level);
    return {LoggerError::kSuccess, std::move(log)};
  } catch (...) {
    return {LoggerError::kInvalidArgument, nullptr};
  }
}

}  // namespace logger
