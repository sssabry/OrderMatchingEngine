# OrderMatchingEngine

Order matching engine with concurrent processing and live market simulation implemented in C++.

## To Build

1. **Generate build files using CMake:**

    ```sh
    cmake -S . -B build
    ```

2. **Build the project:**

    ```sh
    cmake --build build
    ```

## To Run

- **Run the C++ Server:**

    ```sh
    ./build/Debug/OrderEngine.exe
    ```

- **Run the Python Client:**

    ```sh
    python client.py
    ```

## Project Structure

- `main.cpp`: Entry point for the application.
- `OrderBook.cpp`, `OrderBook.h`: Core logic for managing the order book.
- `Order.cpp`, `Order.h`: Definitions for order-related data structures.
- `Server.cpp`: Server-side implementation, handling client connections and order processing.
- `client.py`: Python client for interacting with the server.

## Current Planned Additional Features

- **Visualization & UI:** Develop a Python-based user interface with market visualization to enhance user interaction and experience.
- **FPGA Extension:** Explore FPGA-based processing to speed up the order matching engine and practice HDL implementation.
