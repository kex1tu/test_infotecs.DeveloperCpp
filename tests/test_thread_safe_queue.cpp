// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

#include <atomic>
#include <cstddef>
#include <thread>
#include <unordered_set>

#include "tests.hpp"
#include "thread_safe_queue.hpp"
// проверка базовой функциональности + try_pop + wait_and_pop +
// push(T&&) + clear
bool test_1() {
  logger::ThreadSafeQueue<int> queue;
  int i = 0;
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(queue.size(), 0);
  queue.push(++i);
  ASSERT_TRUE(!queue.empty());
  ASSERT_EQ(queue.size(), static_cast<size_t>(i));
  queue.push(++i);
  ASSERT_EQ(queue.size(), static_cast<size_t>(i));

  auto value = queue.try_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 1);
  ASSERT_EQ(queue.size(), 1);

  value = queue.wait_and_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 2);
  ASSERT_EQ(queue.size(), 0);
  ASSERT_TRUE(queue.empty());

  queue.push(5);
  value = queue.try_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 5);

  queue.push(6);
  queue.push(7);
  queue.push(8);
  ASSERT_EQ(queue.size(), 3);
  queue.clear();
  ASSERT_TRUE(queue.empty());
  ASSERT_EQ(queue.size(), 0);
  ASSERT_TRUE(!queue.try_pop().has_value());

  return true;
}
// всё что со stop
bool test_2() {
  logger::ThreadSafeQueue<int> queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);
  queue.stop();
  auto value = queue.try_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 1);
  ASSERT_EQ(queue.size(), 2);

  queue.stop();
  ASSERT_EQ(queue.push(5), false);

  value = queue.wait_and_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 2);
  ASSERT_EQ(queue.size(), 1);

  return true;
}
// проверка что wait_and_pop() блокируется и потом по cv просыпается
bool test_3() {
  logger::ThreadSafeQueue<int> queue;
  std::optional<int> result = 42;  // любое начальное значение
  std::thread consumer([&queue, &result]() { result = queue.wait_and_pop(); });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  queue.stop();
  consumer.join();
  ASSERT_TRUE(!result.has_value());
  ASSERT_TRUE(queue.empty());

  return true;
}
// добавляем элементы потом останавливаем и достаем все значения
bool test_4() {
  logger::ThreadSafeQueue<int> queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);
  queue.stop();

  ASSERT_EQ(queue.push(4), false);
  ASSERT_EQ(queue.size(), 3);

  auto value = queue.try_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 1);
  ASSERT_EQ(queue.size(), 2);

  value = queue.wait_and_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 2);
  ASSERT_EQ(queue.size(), 1);

  value = queue.wait_and_pop();
  ASSERT_TRUE(value.has_value());
  ASSERT_EQ(value.value(), 3);
  ASSERT_EQ(queue.size(), 0);
  ASSERT_TRUE(queue.empty());

  value = queue.wait_and_pop();
  ASSERT_TRUE(!value.has_value());

  value = queue.try_pop();
  ASSERT_TRUE(!value.has_value());

  return true;
}
// проверка с move-only типами
bool test_5() {
  logger::ThreadSafeQueue<std::unique_ptr<int>> queue;
  auto ptr1 = std::make_unique<int>(100);
  auto ptr2 = std::make_unique<int>(200);

  ASSERT_TRUE(queue.push(std::move(ptr1)));
  ASSERT_TRUE(queue.push(std::move(ptr2)));
  ASSERT_EQ(queue.size(), 2);

  auto val1 = queue.try_pop();
  ASSERT_TRUE(val1.has_value());
  ASSERT_EQ(*val1.value(), 100);
  auto val2 = queue.wait_and_pop();
  ASSERT_TRUE(val2.has_value());
  ASSERT_EQ(*val2.value(), 200);
  ASSERT_TRUE(queue.empty());
  return true;
}

// несколько потоков на пробуждение
bool test_6() {
  logger::ThreadSafeQueue<int> queue;
  constexpr size_t kConsNum = 4;
  std::vector<std::thread> consumers(kConsNum);
  std::vector<std::optional<int>> results(kConsNum);
  for (size_t i = 0; i < kConsNum; ++i) {
    consumers[i] = std::thread(
        [&queue, &results, i]() { results[i] = queue.wait_and_pop(); });
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  queue.stop();
  for (auto& t : consumers) {
    if (t.joinable()) {
      t.join();
    }
  }
  for (size_t i = 0; i < kConsNum; ++i) {
    ASSERT_TRUE(!results[i].has_value());
  }
  return true;
}

// несколько потоков на пробуждение, но с данными
bool test_7() {
  logger::ThreadSafeQueue<size_t> queue;
  constexpr size_t kConsNum = 4;
  std::vector<std::thread> consumers(kConsNum);
  std::vector<std::optional<size_t>> results(kConsNum);
  for (size_t i = 0; i < kConsNum; ++i) {
    consumers[i] = std::thread(
        [&queue, &results, i]() { results[i] = queue.wait_and_pop(); });
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  for (size_t i = 0; i < kConsNum; ++i) {
    queue.push(i);
  }

  queue.stop();
  for (auto& t : consumers) {
    if (t.joinable()) {
      t.join();
    }
  }
  std::unordered_set<size_t> st;
  for (size_t i = 0; i < kConsNum; ++i) {
    ASSERT_TRUE(results[i].has_value());
    ASSERT_TRUE(results[i].value() < kConsNum);
    st.insert(results[i].value());
  }

  ASSERT_EQ(st.size(), kConsNum);
  return true;
}
// один поток много пишет другой много читает
bool test_8() {
  logger::ThreadSafeQueue<size_t> queue;
  constexpr size_t kCount = 1000;
  std::vector<size_t> consumed_values(kCount);

  std::thread consumer([&queue, &consumed_values]() {
    for (size_t i = 0; i < kCount; ++i) {
      auto value = queue.wait_and_pop();
      if (value.has_value()) {
        consumed_values[i] = value.value();
      }
    }
  });

  std::thread producer([&queue]() {
    for (size_t i = 0; i < kCount; ++i) {
      queue.push(i);
    }
  });

  producer.join();
  consumer.join();
  ASSERT_EQ(consumed_values.size(), kCount);
  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(consumed_values[i], i);
  }

  queue.stop();
  return true;
}
// много пишут и много читают
bool test_9() {
  logger::ThreadSafeQueue<size_t> queue;

  constexpr size_t kCount = 4;
  constexpr size_t kItemsPerThread = 10000;
  constexpr size_t kTotalItems = kCount * kItemsPerThread;
  std::atomic<size_t> consumed_count = 0;
  std::atomic<size_t> consumed_sum = 0;
  std::vector<std::thread> consumers(kCount);
  for (size_t i = 0; i < kCount; ++i) {
    consumers[i] = std::thread([&queue, &consumed_count, &consumed_sum]() {
      auto value = queue.wait_and_pop();
      while (value.has_value()) {
        consumed_sum += value.value();
        ++consumed_count;
        value = queue.wait_and_pop();
      }
    });
  }
  std::vector<std::thread> producers(kCount);
  for (size_t i = 0; i < kCount; ++i) {
    producers[i] = std::thread([&queue, i]() {
      for (size_t j = 0; j < kItemsPerThread; ++j) {
        queue.push((i * kItemsPerThread) + j);
      }
    });
  }
  for (auto& t : producers) {
    t.join();
  }
  queue.stop();
  for (auto& t : consumers) {
    t.join();
  }

  ASSERT_EQ(consumed_count.load(), kTotalItems);
  ASSERT_EQ(consumed_sum.load(), (kTotalItems * (kTotalItems - 1)) / 2);
  return true;
}

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Тесты: \n";
  std::cout << "==========================================\n";
  RUN_TEST(test_1);
  RUN_TEST(test_2);
  RUN_TEST(test_3);
  RUN_TEST(test_4);
  RUN_TEST(test_5);
  RUN_TEST(test_6);
  RUN_TEST(test_7);
  RUN_TEST(test_8);
  RUN_TEST(test_9);

  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}