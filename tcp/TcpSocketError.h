#pragma once
#include <exception>
#include <format>
#include <string>
#include <winsock2.h>

class TcpSocketError final : public std::exception {
public:
  explicit TcpSocketError(const std::string& funcName) : message(
    std::format("{} failed. WSA last error code: {}", funcName, WSAGetLastError())) {
  }

  TcpSocketError(const std::string& funcName, int errorCode) : message(
    std::format("{} failed. Error code: {}", funcName, errorCode)) {
  }

  [[nodiscard]] const char* what() const noexcept override {
    return message.c_str();
  }

private:
  std::string message;
};