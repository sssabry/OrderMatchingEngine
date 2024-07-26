cmake_minimum_required(VERSION 3.10)
project(OrderEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
set(SOURCES
    OrderEngine.cpp
)

add_executable(OrderEngine ${SOURCES})

# target_link_libraries(OrderBookApp <library>)

enable_testing()

# Update: # add_test(NAME test_name COMMAND executable_name)
