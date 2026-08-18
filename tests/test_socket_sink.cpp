// Copyright (C) 2026 Grigoriy Mikheyev. All rights reserved.
// Distributed under MIT license or project terms.

// NOLINTBEGIN
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "logger.hpp"
#include "logger_error.hpp"
#include "logger_factories.hpp"
#include "socket_sink.hpp"
#include "tests.hpp"
#include "utilities.hpp"

namespace {

class LocalListener {
 public:
  LocalListener() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
      return;
    }

    int opt = 1;
    ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <
        0) {
      close_all();
      return;
    }

    socklen_t addr_len = sizeof(addr);
    if (::getsockname(listen_fd_, reinterpret_cast<sockaddr*>(&addr),
                      &addr_len) < 0) {
      close_all();
      return;
    }

    port_ = ntohs(addr.sin_port);

    if (::listen(listen_fd_, 1) < 0) {
      close_all();
      return;
    }
  }

  ~LocalListener() { close_all(); }

  LocalListener(const LocalListener&) = delete;
  LocalListener& operator=(const LocalListener&) = delete;
  LocalListener(LocalListener&&) = delete;
  LocalListener& operator=(LocalListener&&) = delete;

  bool is_valid() const noexcept { return listen_fd_ >= 0; }
  int get_port() const noexcept { return port_; }

  bool accept_client() {
    if (listen_fd_ < 0) {
      return false;
    }
    if (client_fd_ >= 0) {
      ::close(client_fd_);
      client_fd_ = -1;
    }
    client_fd_ = ::accept(listen_fd_, nullptr, nullptr);
    return client_fd_ >= 0;
  }

  std::string read_all() const {
    if (client_fd_ < 0) {
      return {};
    }
    std::string result;
    std::array<char, 1024> buffer{};
    while (true) {
      const ssize_t bytes =
          ::recv(client_fd_, buffer.data(), buffer.size(), MSG_DONTWAIT);
      if (bytes <= 0) {
        break;
      }
      result.append(buffer.data(), static_cast<std::size_t>(bytes));
    }
    return result;
  }

  void close_client() {
    if (client_fd_ >= 0) {
      ::close(client_fd_);
      client_fd_ = -1;
    }
  }

  void close_all() {
    close_client();
    if (listen_fd_ >= 0) {
      ::close(listen_fd_);
      listen_fd_ = -1;
    }
  }

 private:
  int listen_fd_{-1};
  int client_fd_{-1};
  int port_{0};
};

}  // namespace

bool test_1() {
  LocalListener listener;
  ASSERT_TRUE(listener.is_valid());

  logger::SocketSink sink;
  ASSERT_TRUE(!sink.is_open());

  const auto err = sink.connect("127.0.0.1", listener.get_port());
  ASSERT_EQ(err, logger::LoggerError::kSuccess);
  ASSERT_TRUE(sink.is_open());
  ASSERT_TRUE(listener.accept_client());

  const std::string test_payload = "Hello TCP Server!\n";
  const auto write_err = sink.write(test_payload);
  ASSERT_EQ(write_err, logger::LoggerError::kSuccess);

  sink.close();
  ASSERT_TRUE(!sink.is_open());

  const std::string received = listener.read_all();
  ASSERT_EQ(received, test_payload);

  return true;
}

bool test_2() {
  LocalListener listener;
  ASSERT_TRUE(listener.is_valid());

  auto [err, log] = logger::make_socket_logger("127.0.0.1", listener.get_port(),
                                               logger::LogLevel::kInfo);
  ASSERT_EQ(err, logger::LoggerError::kSuccess);
  ASSERT_TRUE(log != nullptr);
  ASSERT_TRUE(listener.accept_client());

  const auto log_err1 =
      log->log_message(logger::LogLevel::kInfo, "Exact formatted test message");
  ASSERT_EQ(log_err1, logger::LoggerError::kSuccess);

  const auto log_err2 =
      log->log_message(logger::LogLevel::kError, "Second error message");
  ASSERT_EQ(log_err2, logger::LoggerError::kSuccess);

  log->close();

  const std::string received = listener.read_all();
  ASSERT_TRUE(received.find("[INFO] Exact formatted test message\n") !=
              std::string::npos);
  ASSERT_TRUE(received.find("[ERROR] Second error message\n") !=
              std::string::npos);

  return true;
}

bool test_3() {
  int closed_port = 0;
  {
    LocalListener temp;
    ASSERT_TRUE(temp.is_valid());
    closed_port = temp.get_port();
  }

  logger::SocketSink sink;
  const auto err = sink.connect("127.0.0.1", closed_port);
  ASSERT_EQ(err, logger::LoggerError::kSinkError);
  ASSERT_TRUE(!sink.is_open());

  auto [factory_err, log] =
      logger::make_socket_logger("127.0.0.1", closed_port);
  ASSERT_EQ(factory_err, logger::LoggerError::kSinkError);
  ASSERT_TRUE(log == nullptr);

  return true;
}

bool test_4() {
  logger::SocketSink sink;

  ASSERT_EQ(sink.connect("999.999.999.999", 8080),
            logger::LoggerError::kInvalidArgument);
  ASSERT_EQ(sink.connect("", 8080), logger::LoggerError::kInvalidArgument);
  ASSERT_EQ(sink.connect("not_an_ip", 8080),
            logger::LoggerError::kInvalidArgument);
  ASSERT_EQ(sink.connect("127.0.0.1.1", 8080),
            logger::LoggerError::kInvalidArgument);

  ASSERT_EQ(sink.connect("127.0.0.1", 0),
            logger::LoggerError::kInvalidArgument);
  ASSERT_EQ(sink.connect("127.0.0.1", -1),
            logger::LoggerError::kInvalidArgument);
  ASSERT_EQ(sink.connect("127.0.0.1", 70000),
            logger::LoggerError::kInvalidArgument);
  ASSERT_TRUE(!sink.is_open());

  auto [err_ip, log_ip] = logger::make_socket_logger("999.999.999.999", 8080);
  ASSERT_EQ(err_ip, logger::LoggerError::kInvalidArgument);
  ASSERT_TRUE(log_ip == nullptr);

  auto [err_empty, log_empty] = logger::make_socket_logger("", 8080);
  ASSERT_EQ(err_empty, logger::LoggerError::kInvalidArgument);
  ASSERT_TRUE(log_empty == nullptr);

  auto [err_port, log_port] = logger::make_socket_logger("127.0.0.1", 0);
  ASSERT_EQ(err_port, logger::LoggerError::kInvalidArgument);
  ASSERT_TRUE(log_port == nullptr);

  return true;
}

bool test_5() {
  logger::SocketSink sink;
  ASSERT_TRUE(!sink.is_open());
  ASSERT_EQ(sink.write("msg before connect"), logger::LoggerError::kSinkError);

  LocalListener listener;
  ASSERT_TRUE(listener.is_valid());

  ASSERT_EQ(sink.connect("127.0.0.1", listener.get_port()),
            logger::LoggerError::kSuccess);
  ASSERT_TRUE(sink.is_open());

  sink.close();
  ASSERT_TRUE(!sink.is_open());
  ASSERT_EQ(sink.write("msg after close"), logger::LoggerError::kSinkError);

  return true;
}

bool test_6() {
  LocalListener listener1;
  LocalListener listener2;
  ASSERT_TRUE(listener1.is_valid());
  ASSERT_TRUE(listener2.is_valid());

  logger::SocketSink sink;

  ASSERT_EQ(sink.connect("127.0.0.1", listener1.get_port()),
            logger::LoggerError::kSuccess);
  ASSERT_TRUE(sink.is_open());
  ASSERT_TRUE(listener1.accept_client());
  ASSERT_EQ(sink.write("Message to server1\n"), logger::LoggerError::kSuccess);
  ASSERT_TRUE(listener1.read_all().find("Message to server1") !=
              std::string::npos);

  ASSERT_EQ(sink.connect("127.0.0.1", listener2.get_port()),
            logger::LoggerError::kSuccess);
  ASSERT_TRUE(sink.is_open());
  ASSERT_TRUE(listener2.accept_client());
  ASSERT_EQ(sink.write("Message to server2\n"), logger::LoggerError::kSuccess);
  ASSERT_TRUE(listener2.read_all().find("Message to server2") !=
              std::string::npos);

  sink.close();
  ASSERT_TRUE(!sink.is_open());
  ASSERT_EQ(sink.connect("127.0.0.1", listener1.get_port()),
            logger::LoggerError::kSuccess);
  ASSERT_TRUE(sink.is_open());
  ASSERT_TRUE(listener1.accept_client());
  ASSERT_EQ(sink.write("Second message to server1\n"),
            logger::LoggerError::kSuccess);
  ASSERT_TRUE(listener1.read_all().find("Second message to server1") !=
              std::string::npos);

  sink.close();
  return true;
}

int main() {
  int passed = 0;
  int total = 0;

  std::cout << "Тесты SocketSink\n";
  std::cout << "==========================================\n";
  RUN_TEST(test_1);
  RUN_TEST(test_2);
  RUN_TEST(test_3);
  RUN_TEST(test_4);
  RUN_TEST(test_5);
  RUN_TEST(test_6);

  std::cout << "==========================================\n";
  std::cout << "Итого: " << passed << " из " << total << " тестов пройдено.\n";

  return (passed == total) ? 0 : 1;
}
// NOLINTEND