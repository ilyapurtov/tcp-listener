#pragma once

#include "WinsockInit.h"

#include <bits/this_thread_sleep.h>
#include <chrono>
#include <memory>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

class TcpSocket {
public:
  TcpSocket(const std::string &ip, const std::string &port) {

    int iResult = 0;

    addrinfo address{};
    ZeroMemory(&address, sizeof(address));
    address.ai_family = AF_INET;
    address.ai_socktype = SOCK_STREAM;
    address.ai_protocol = IPPROTO_TCP;
    address.ai_flags = AI_PASSIVE;

    addrinfo *rawAddr = nullptr;
    if (iResult = getaddrinfo(ip.c_str(), port.c_str(), &address, &rawAddr);
        iResult != 0) {
      throw TcpSocketError("getaddrinfo", iResult);
    }
    mAddr.reset(rawAddr);

    mSocket = socket(mAddr->ai_family, mAddr->ai_socktype, mAddr->ai_protocol);
    if (mSocket == INVALID_SOCKET) {
      throw TcpSocketError("socket");
    }
  }

  static void sendMessage(const std::string& message, const SOCKET dest) {
    // sending message length
    u_short length = htons(message.length());
    if (int iResult = send(dest, reinterpret_cast<char *>(&length), sizeof(length), 0); iResult != SOCKET_ERROR) {

      // sending message itself
      iResult = send(dest, message.data(), message.size(), 0);
      if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == 10054) { // disconnect
          return;
        }
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
          sleep();
        }
        throw TcpSocketError("send");
      }
    } else {
      if (WSAGetLastError() == 10054) { // disconnect
        return;
      }
      throw TcpSocketError("send");
    }
  }

  void sendMessage(const std::string& message) const {
    sendMessage(message, this->mSocket);
  }

  static std::optional<std::string> receiveMessage(const SOCKET from) {
    u_short size;
    if (int bytesReceived = recv(from, reinterpret_cast<char*>(&size), sizeof(size), MSG_WAITALL); bytesReceived > 0) {
      size = ntohs(size);
      char message[size];
      bytesReceived = recv(from, message, size, 0);
      if (bytesReceived > 0) {
        return std::string(message, size);
      }
      if (bytesReceived == 0 || WSAGetLastError() == 10054) { // disconnect
        return std::nullopt;
      }
      if (WSAGetLastError() == WSAEWOULDBLOCK) {

      }
      throw TcpSocketError("recv");
    } else {
      if (bytesReceived == 0 || WSAGetLastError() == 10054) { // disconnect
        return std::nullopt;
      }
      throw TcpSocketError("recv");
    }
  }

  std::optional<std::string> receiveMessage() const {
    return receiveMessage(this->mSocket);
  }

  static void makeNonBlocking(const SOCKET socket) {
    u_long mode = 1;
    ioctlsocket(socket, FIONBIO, &mode);
  }

  virtual ~TcpSocket() {
    if (mSocket != INVALID_SOCKET) {
      closesocket(mSocket);
      mSocket = INVALID_SOCKET;
    }
  }
  TcpSocket(const TcpSocket &other) = delete;
  TcpSocket(TcpSocket &&other) = delete;

protected:
  SOCKET mSocket = INVALID_SOCKET;
  std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> mAddr = {nullptr,
                                                              &freeaddrinfo};

  static void sleep() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

private:
  WinsockInit mWinsockInit;
};
