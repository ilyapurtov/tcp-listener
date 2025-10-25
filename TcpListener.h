#pragma once

#include <format>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

class TcpListener {
public:
  TcpListener(const std::string& ip, const std::string& port);
  void run(const std::function<void()>& onRun = [] {});
  void stop();
  ~TcpListener();

private:
  static bool initialized;
  std::atomic<bool> running = false;
  addrinfo* addr = nullptr;
  SOCKET serverSocket;
  std::vector<SOCKET> clients;
  std::mutex clientsMutex;
  std::vector<std::thread> clientThreads;

  void mRun();
  void mHandleClient(SOCKET client);
};

class TcpListenerError final : public std::exception {
public:
  explicit TcpListenerError(const std::string& funcName) : message(
    std::format("{} failed. WSA last error code: {}", funcName, WSAGetLastError())) {
  }

  TcpListenerError(const std::string& funcName, int errorCode) : message(
    std::format("{} failed. Error code: {}", funcName, errorCode)) {
  }

  [[nodiscard]] const char* what() const noexcept override {
    return message.c_str();
  }

private:
  std::string message;
};
