# CMake minimum version and project name
cmake_minimum_required(VERSION 3.10)
project(OrderBookApp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
set(SOURCES
    main.cpp
    # Add other source files here
)

# Create executable
add_executable(OrderBookApp ${SOURCES})

# Link libraries if needed
# target_link_libraries(OrderBookApp <library>)

# Enable testing (optional)
enable_testing()

# Add tests if you have any
# add_test(NAME test_name COMMAND executable_name)
