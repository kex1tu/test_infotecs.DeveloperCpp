// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "log_level.hpp"

namespace app {

/**
 * \brief Режим вывода сообщений клиентского приложения.
 */
enum class OutputMode : std::uint8_t {
  kFile,  ///< Вывод логов в локальный файл на диске
  kSocket,  ///< Передача логов по сети через TCP-сокет
};

/**
 * \brief Конфигурация параметров запуска клиентского приложения.
 *
 * Содержит параметры, извлеченные из аргументов командной строки (CLI).
 */
struct AppConfig {
  OutputMode mode{OutputMode::kFile};  ///< Целевой режим вывода
  std::string filepath;  ///< Путь к файлу (для режима kFile)
  std::string remote_addr;  ///< IPv4-адрес сервера (для режима kSocket)
  uint16_t port = 0;  ///< TCP-порт сервера (для режима kSocket)
  logger::LogLevel min_level =
      logger::LogLevel::kDebug;  ///< Минимальный уровень логирования
};

/**
 * \brief Разбирает аргументы командной строки и формирует конфигурацию.
 *
 * Поддерживаемые форматы аргументов:
 * - --file <filename> [min_level]
 * - --socket <address> <port> [min_level]
 *
 * \param[in] argc Количество аргументов командной строки.
 * \param[in] argv Массив указателей на строки аргументов.
 * \return std::optional со структурой AppConfig при корректных параметрах,
 *         или std::nullopt при ошибке синтаксиса или невалидных значениях.
 * \pre argv != nullptr
 */
std::optional<AppConfig> parse_cli_args(int argc, char** argv) noexcept;

/**
 * \brief Преобразует строковое представление TCP-порта в числовое значение.
 *
 * \param[in] str Строка, содержащая десятичное число порта.
 * \param[out] port Ссылка для записи распарсенного значения порта.
 * \return true если порт успешно распарсен и находится в диапазоне [1..65535],
 *         false при наличии нечисловых символов или выходе из диапазона.
 * \post При возврате true значение port находится в диапазоне [1..65535].
 */
bool parse_port(std::string_view str, uint16_t& port) noexcept;

/**
 * \brief Выводит справочную информацию о правилах запуска приложения.
 *
 * \param[in] program_name Имя исполняемого файла (argv[0]).
 */
void print_usage(std::string_view program_name) noexcept;

/**
 * \brief Разобранный элемент пользовательского ввода из консоли.
 *
 * Представляет пару: целевой уровень логирования и текст сообщения.
 */
struct LogItem {
  logger::LogLevel level;  ///< Уровень важности сообщения
  std::string message;  ///< Текст сообщения без тега уровня
};

/**
 * \brief Разбирает строку ввода, извлекая опциональный тег [LEVEL] и текст.
 *
 * Если строка начинается с тега вида "[INFO] message", уровень извлекается
 * из тега, а остаток строки становится сообщением. В противном случае
 * используется default_level.
 *
 * \param[in] input Входная строка пользовательского ввода.
 * \param[in] default_level Уровень логирования по умолчанию при отсутствии
 * тега. \return Структура LogItem с определенным уровнем и очищенным
 * сообщением.
 */
LogItem parse_input(
    std::string_view input,
    logger::LogLevel default_level = logger::LogLevel::kDebug) noexcept;

}  // namespace app