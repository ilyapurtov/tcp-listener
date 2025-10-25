#include "TcpListener.h"

#include <algorithm>
#include <format>
#include <iostream>

bool TcpListener::initialized = false;

TcpListener::TcpListener(const std::string& ip, const std::string& port) {
  int iResult = 0;
  if (!initialized) {
    WSAData wsaData{};
    if (iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); iResult != 0) {
      throw TcpListenerError("test", iResult);
    }
    initialized = true;
  }

  addrinfo address;
  ZeroMemory(&address, sizeof (address));
  address.ai_family = AF_INET;
  address.ai_socktype = SOCK_STREAM;
  address.ai_protocol = IPPROTO_TCP;
  address.ai_flags = AI_PASSIVE;

  if (iResult = getaddrinfo(ip.c_str(), port.c_str(), &address, &addr); iResult != 0) {
    throw TcpListenerError("getaddrinfo", iResult);
  }

  serverSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if (serverSocket == INVALID_SOCKET) {
    throw TcpListenerError("socket");
  }

  iResult = bind(serverSocket, addr->ai_addr, static_cast<int>(addr->ai_addrlen));
  if (iResult == SOCKET_ERROR) {
    throw TcpListenerError("bind");
  }
  freeaddrinfo(addr);
  addr = nullptr;
}

void TcpListener::run(const std::function<void()>& onRun) {
  if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
    throw TcpListenerError("listen");
  }
  running = true;
  onRun();
  mRun();
}

void TcpListener::mRun() {
  while (running) {
    const SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (WSAGetLastError() == 10004) {
      break;
    }
    if (clientSocket == INVALID_SOCKET) {
      throw TcpListenerError("accept");
    }
    std::cout << "new client connected " << clientSocket << std::endl;
    clientThreads.emplace_back([this, clientSocket] { mHandleClient(clientSocket); });
  }
}

void TcpListener::mHandleClient(const SOCKET client) {
  {
    std::lock_guard guard(clientsMutex);
    clients.push_back(client);
  }
  while (running) {
    char message[512];
    if (const int bytesReceived = recv(client, message, 512, 0); bytesReceived > 0) {
      std::lock_guard guard(clientsMutex);
      for (const auto& socket : clients) {
        if (socket == client) continue;
        if (const int sendResult = send(socket, message, bytesReceived, 0); sendResult == SOCKET_ERROR) {
          throw TcpListenerError("send");
        }
      }
    } else if (bytesReceived == 0) {
      break;
    } else {
      throw TcpListenerError("recv");
    }
  }

  closesocket(client);
  {
    std::lock_guard guard(clientsMutex);
    const auto removed = std::ranges::remove(clients.begin(), clients.end(), client);
    clients.erase(removed.begin(), removed.end());
  }
  std::cout << "client disconnected" << std::endl;
}

void TcpListener::stop() {
  running = false;
  for (auto& thread : clientThreads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  clientThreads.clear();
  if (initialized) {
    WSACleanup();
    initialized = false;
  }
  if (addr) {
    freeaddrinfo(addr);
    addr = nullptr;
  }
  if (serverSocket != INVALID_SOCKET) {
    closesocket(serverSocket);
  }
}

TcpListener::~TcpListener() {
  stop();
}
