#include <iostream>

#include "tcp/ClientManager.h"
#include "tcp/TcpListener.h"
#include <format>

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "8080"

class Client {
public:
  SOCKET socket;
  std::string name;

  bool operator==(const Client &rhs) const { return socket == rhs.socket; }
};

std::unique_ptr<TcpListener> server = nullptr;
ClientManager<Client> clientManager;

BOOL WINAPI ConsoleHandler(const DWORD signal) {
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT || signal == CTRL_SHUTDOWN_EVENT ||
      signal == CTRL_BREAK_EVENT) {
    std::cout << "stopping..." << std::endl;
    server->stop();
    return TRUE;
  }
  return FALSE;
}

void broadcast(const Client &from, const std::string& message) {
  clientManager.forEach([&](const Client &client) {
    if (client == from) return;
    server->sendMessage(message, client.socket);
  });
}

void handleClient(const SOCKET clientSocket) {
  const auto nickname = server->receiveMessage(clientSocket);
  if (!nickname.has_value()) {
    return; // disconnected
  }
  const Client thisClient{clientSocket, nickname.value()};
  clientManager.addClient(thisClient);

  auto formatted = std::format("new user {} joined", thisClient.name);
  std::cout << formatted << std::endl;
  broadcast(thisClient, formatted);

  while (server->isRunning()) {
    if (const auto message = server->receiveMessage(clientSocket); message.has_value()) {
      formatted = std::format("[{}]: {}", thisClient.name, message.value());
      std::cout << formatted << std::endl;
      broadcast(thisClient, formatted);
    } else {
      break; // disconnected
    }
  }

  if (clientSocket != INVALID_SOCKET) {
    closesocket(clientSocket);
  }
  clientManager.removeClient(thisClient);

  formatted = std::format("user {} left", thisClient.name);
  std::cout << formatted << std::endl;
  broadcast(thisClient, formatted);
}

int main(const int argc, char *argv[]) {

  SetConsoleCtrlHandler(ConsoleHandler, TRUE);

  const char *ip = argc > 1 ? argv[1] : DEFAULT_IP;
  const char *port = argc > 2 ? argv[2] : DEFAULT_PORT;

  server = std::make_unique<TcpListener>(ip, port);
  server->run(
      [&ip, &port] {
        std::cout << std::format("Server is running on {}:{}", ip, port) << std::endl;
      },
      &handleClient);

  return 0;
}