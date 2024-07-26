# OrderMatchingEngine
Order matching engine, concurrent processing & live market simulation in C++

## To Build:
'''
    cmake -S . -B build
    cmake --build build
'''

To run C++ Server:
'''
    ./build/Debug/OrderEngine.exe
'''
To run a python client:
'''
    python client.py
'''

## Current Planned Additional Features:
- Visualization of market & python-based user interface -- turn it into an entire application?
- FPGA-based extension to speed up processing and practice HDL implementation of driver code
- explosion of OrderBook.cpp, into header files + MAKEFILE to implement compilation
