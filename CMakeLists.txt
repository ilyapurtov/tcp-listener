cmake_minimum_required(VERSION 4.0)
project(winsockets)

set(CMAKE_CXX_STANDARD 20)

add_executable(winsockets main.cpp
        TcpListener.cpp)
target_link_libraries(winsockets Ws2_32.lib)