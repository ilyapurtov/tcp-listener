#pragma once

#include "TcpSocket.h"

#include <format>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>

class TcpListener final : public TcpSocket {
public:
  TcpListener(const std::string &ip, const std::string &port);
  void run(
      const std::function<void()> &onRun = [] {},
      const std::function<void(SOCKET)> &onConnection = [](SOCKET) {});
  void stop();
  ~TcpListener() override;
  bool isRunning();

private:
  std::atomic<bool> running = false;
  std::vector<std::thread> clientThreads;

  void mRun(const std::function<void(SOCKET)>& onConnection);
};
