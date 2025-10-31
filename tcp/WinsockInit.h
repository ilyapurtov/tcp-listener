#pragma once
#include "TcpSocketError.h"

#include <winsock2.h>

class WinsockInit {
public:
  WinsockInit() {
    if (count == 0) {
      WSAData wsaData{};
      if (const int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); iResult != 0) {
        throw TcpSocketError("test", iResult);
      }
    }
    count++;
  }
  ~WinsockInit() {
    count--;
    if (count == 0) {
      WSACleanup();
    }
  }
private:
  inline static int count = 0;
};