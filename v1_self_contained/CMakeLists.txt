cmake_minimum_required(VERSION 3.10)
project(OrderEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OrderEngine OrderEngine.cpp)

if (WIN32)
    target_link_libraries(OrderEngine ws2_32)
endif()
