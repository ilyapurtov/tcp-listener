#include <iostream>

#include "TcpListener.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "8080"

std::unique_ptr<TcpListener> listener = nullptr;

BOOL WINAPI ConsoleHandler(const DWORD signal)
{
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT ||
      signal == CTRL_SHUTDOWN_EVENT || signal == CTRL_BREAK_EVENT)
  {
    std::cout << "stopping..." << std::endl;
    listener->stop();
    return TRUE;
  }
  return FALSE;
}

int main(const int argc, char* argv[]) {

  SetConsoleCtrlHandler(ConsoleHandler, TRUE);

  const char* ip = argc > 1 ? argv[1] : DEFAULT_IP;
  const char* port = argc > 2 ? argv[2] : DEFAULT_PORT;

  listener = std::make_unique<TcpListener>(ip, port);
  listener->run([ip, port] {
    std::cout << std::format("Server is running on {}:{}", ip, port) << std::endl;
  });

  return 0;
}