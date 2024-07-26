cmake_minimum_required(VERSION 3.10)
project(OrderEngine)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    main.cpp
    OrderBook.cpp
    Order.cpp
    Server.cpp
)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(OrderEngine ${SOURCES})

target_link_libraries(OrderEngine ws2_32)  
