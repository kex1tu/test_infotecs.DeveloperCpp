// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace logger {

/**
 * \brief Потокобезопасная очередь с поддержкой корректного завершения (stop).
 *
 * Обеспечивает безопасную передачу данных между потоками по паттерну
 * Producer-Consumer. Поддерживает как блокирующее извлечение (wait_and_pop),
 * так и неблокирующее (try_pop).
 *
 * \tparam T Тип элементов очереди.
 * \note Все публичные методы являются Thread-safe и noexcept.
 */
template <typename T>
class ThreadSafeQueue {
 private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool is_working_ = true;

 public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() { stop(); }

  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue(ThreadSafeQueue&&) = delete;
  ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

  /**
   * \brief Завершает работу очереди и пробуждает все ожидающие потоки.
   *
   * \post is_working_ == false, все потоки в wait_and_pop разблокируются.
   */
  void stop() noexcept {
    {
      std::scoped_lock<std::mutex> lock{mutex_};
      is_working_ = false;
    }
    cv_.notify_all();
  }

  /**
   * \brief Добавляет элемент в конец очереди.
   *
   * \tparam U Универсальный тип добавляемого элемента.
   * \param[in] value Элемент для добавления.
   * \return true при успешном добавлении, false если очередь остановлена.
   */
  template <typename U>
  bool push(U&& value) noexcept {
    try {
      {
        std::scoped_lock<std::mutex> lock{mutex_};
        if (!is_working_) {
          return false;
        }
        queue_.push(std::forward<U>(value));
      }
      cv_.notify_one();
      return true;
    } catch (...) {
      return false;
    }
  }

  /**
   * \brief Извлекает элемент из очереди с блокирующим ожиданием.
   *
   * \return std::optional с элементом, или std::nullopt если очередь
   * остановлена и пуста.
   */
  std::optional<T> wait_and_pop() noexcept {
    try {
      std::unique_lock<std::mutex> lock{mutex_};
      cv_.wait(lock, [this]() { return !queue_.empty() || !is_working_; });

      if (!is_working_ && queue_.empty()) {
        return std::nullopt;
      }
      auto value = std::move(queue_.front());
      queue_.pop();
      return value;
    } catch (...) {
      return std::nullopt;
    }
  }

  /**
   * \brief Пытается извлечь элемент из очереди без блокировки.
   *
   * \return std::optional с элементом при непустой очереди, иначе std::nullopt.
   */
  std::optional<T> try_pop() noexcept {
    try {
      std::scoped_lock<std::mutex> lock{mutex_};
      if (queue_.empty()) {
        return std::nullopt;
      }
      auto value = std::move(queue_.front());
      queue_.pop();
      return value;
    } catch (...) {
      return std::nullopt;
    }
  }

  /**
   * \brief Проверяет, пуста ли очередь.
   *
   * \return true если в очереди нет элементов, иначе false.
   */
  bool empty() const noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    return queue_.empty();
  }

  /**
   * \brief Возвращает текущее количество элементов в очереди.
   *
   * \return Число элементов.
   */
  size_t size() const noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    return queue_.size();
  }

  /**
   * \brief Очищает все элементы из очереди.
   *
   * \post empty() == true.
   */
  void clear() noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    queue_ = {};
  }
};

}  // namespace logger
