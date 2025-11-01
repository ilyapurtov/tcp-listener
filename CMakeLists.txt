cmake_minimum_required(VERSION 4.0)
project(winsockets)

include(FetchContent)
FetchContent_Declare(tcp_lib SOURCE_DIR D:/programming/cpp/tcp_lib)
FetchContent_MakeAvailable(tcp_lib)

set(CMAKE_CXX_STANDARD 23)

add_executable(winsockets main.cpp)
target_link_libraries(winsockets PRIVATE tcp_lib)