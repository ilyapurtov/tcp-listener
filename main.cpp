#include <iostream>

#include "tcp/ClientManager.h"
#include "tcp/TcpListener.h"

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

void handleClient(const SOCKET clientSocket) {
  std::cout << "connections" << std::endl;
  const auto nickname = server->receiveMessage(clientSocket);
  if (!nickname.has_value()) {
    return; // disconnected
  }
  const Client client{clientSocket, nickname.value()};
  clientManager.addClient(client);

  std::cout << std::format("user {} connected", client.name) << std::endl;

  while (server->isRunning()) {
    if (const auto message = server->receiveMessage(clientSocket); message.has_value()) {
      clientManager.forEach([&client, &message](const Client &otherClient) {
        if (otherClient == client)
          return;
        if (server->sendMessage(message.value(), otherClient.socket)) {
          // success
        } else {
          // TODO delete client on "false"
        }
      });
    } else {
      break; // disconnected
    }
  }

  if (clientSocket != INVALID_SOCKET) {
    closesocket(clientSocket);
  }
  clientManager.removeClient(client);

  std::cout << std::format("user {} disconnected", client.name) << std::endl;
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