// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#include "log_level.hpp"
#include "tcp_server.hpp"
#include "unique_fd.hpp"

namespace stats {

using logger::UniqueFd;

struct ParsedLogEntry {
  logger::LogLevel level{logger::LogLevel::kUnknown};
  std::size_t message_length{0};
};

bool parse_port(std::string_view str, uint16_t& port) noexcept;
bool parse_positive_uint64(std::string_view str, uint64_t& out_val) noexcept;
bool is_valid_ipv4_address(std::string_view address) noexcept;

/**
 * \brief разбирает форматированную строку лога и извлекает уровень и длину
 * сообщения.
 *
 * ожидаемый формат: "[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message text"
 *
 * \param[in] line входная строка лога.
 * \return std::optional с разобранной структурой ParsedLogEntry при успехе,
 *         std::nullopt если строка повреждена или уровень не распознан.
 */
[[nodiscard]] std::optional<ParsedLogEntry> parse_log_line(
    std::string_view line) noexcept;

/**
 * \brief разбирает аргументы командной строки и формирует конфигурацию сервера.
 *
 * формат аргументов:
 * <address> <port> <n_messages> <timeout_sec>
 *
 * \param[in] argc количество аргументов командной строки.
 * \param[in] argv массив указателей на строки аргументов.
 * \return std::optional со структурой ServerConfig при корректных параметрах,
 *         или std::nullopt при ошибке синтаксиса или невалидных значениях.
 * \pre argv != nullptr
 */
std::optional<ServerConfig> parse_cli_args(int argc,
                                           char* const argv[]) noexcept;

void print_usage(std::string_view program_name) noexcept;

}  // namespace stats
