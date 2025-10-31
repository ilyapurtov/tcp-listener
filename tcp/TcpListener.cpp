#include "TcpListener.h"

#include "TcpSocketError.h"

#include <algorithm>
#include <format>
#include <iostream>

TcpListener::TcpListener(const std::string &ip, const std::string &port)
    : TcpSocket(ip, port) {
  const int iResult =
      bind(mSocket, mAddr->ai_addr, static_cast<int>(mAddr->ai_addrlen));
  if (iResult == SOCKET_ERROR) {
    throw TcpSocketError("bind");
  }
}

void TcpListener::run(const std::function<void()> &onRun,
                      const std::function<void(SOCKET)> &onConnection) {
  if (listen(mSocket, SOMAXCONN) == SOCKET_ERROR) {
    throw TcpSocketError("listen");
  }
  running = true;
  onRun();
  mRun(onConnection);
}

void TcpListener::mRun(const std::function<void(SOCKET)> &onConnection) {
  while (running) {
    const SOCKET clientSocket = accept(mSocket, nullptr, nullptr);
    if (WSAGetLastError() == 10004) {
      break;
    }
    if (WSAGetLastError() == WSAEWOULDBLOCK) {

    }
    if (clientSocket == INVALID_SOCKET) {
      throw TcpSocketError("accept");
    }
    clientThreads.emplace_back(onConnection, clientSocket);
  }
}

void TcpListener::stop() {
  running = false;
  for (auto &thread : clientThreads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  clientThreads.clear();
}

TcpListener::~TcpListener() { stop(); }
bool TcpListener::isRunning() {
  return running;
}
