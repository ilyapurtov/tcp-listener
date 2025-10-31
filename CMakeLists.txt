cmake_minimum_required(VERSION 4.0)
project(winsockets)

set(CMAKE_CXX_STANDARD 20)

add_executable(winsockets main.cpp
        tcp/TcpListener.cpp
        tcp/TcpSocket.h
        tcp/TcpSocketError.h
        tcp/WinsockInit.h
        tcp/ClientManager.h
        tcp/ClientManager.h)
target_link_libraries(winsockets Ws2_32.lib)