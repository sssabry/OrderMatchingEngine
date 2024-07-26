#include "Order.h"
#include <iostream>

void Order::print() const {
    std::cout << "Order ID: " << id
              << ", Type: " << (type == OrderType::Market ? "Market" : "Limit")
              << ", Side: " << (side == Side::Buy ? "Buy" : "Sell")
              << ", Price: " << price
              << ", Quantity: " << quantity
              << std::endl;
}
