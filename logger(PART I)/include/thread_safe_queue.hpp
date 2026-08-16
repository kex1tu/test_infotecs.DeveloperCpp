#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace logger {
template <typename T>
// потокобезоапасная очередь
// все отмечено как noexcept и таковым является
// конструкторы и перемещение запрещены
// nullopt используется для сигнализирования об окончании работы или ошибке при
// попытке достать элемент

class ThreadSafeQueue {
 private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool is_working_ = true;

 public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() { stop(); }
  // очередь в одном экземпялре не перемещаем
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue(ThreadSafeQueue&&) = delete;
  ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

  void stop() noexcept {
    {
      std::scoped_lock<std::mutex> lock{mutex_};
      is_working_ = false;
    }
    // чтобы все потоки узнали о изменении состояния
    cv_.notify_all();
  }

  bool push(T&& value) noexcept {
    try {
      {
        std::scoped_lock<std::mutex> lock{mutex_};
        if (!is_working_) {
          return false;
        }
        queue_.push(std::move(value));
      }
      cv_.notify_one();  // пробуждаем один поток чтобы все потоки не стремились
                         // сразу к очереди

      return true;
    } catch (...) {
      return false;
    }
  }

  bool push(const T& value) noexcept {
    try {
      {
        std::scoped_lock<std::mutex> lock{mutex_};
        if (!is_working_) {
          return false;
        }
        queue_.push(value);
      }
      cv_.notify_one();  // см. выше в push(T&& value)
      return true;
    } catch (...) {
      return false;
    }
  }

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

  bool empty() const noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    return queue_.empty();
  }
  size_t size() const noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    return queue_.size();
  }
  void clear() noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    while (!queue_.empty()) {
      queue_.pop();
    }
  }
};
}  // namespace logger
