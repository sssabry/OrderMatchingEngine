#include "OrderBook.h"
#include <iostream>
#include <string>
#include <winsock2.h>

void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(order.side == Side::Buy ? buyOrdersMutex : sellOrdersMutex);
    
    if (order.side == Side::Buy) {
        buyOrdersMap[order.price].push(order);
    } else {
        sellOrdersMap[order.price].push(order);
    }

    // Parallel call to matchOrders: (non-blocking execution)
    matchOrders();
}

void OrderBook::matchOrders() {
    std::lock_guard<std::mutex> buyLock(buyOrdersMutex);
    std::lock_guard<std::mutex> sellLock(sellOrdersMutex);

    while (!buyOrdersMap.empty() && !sellOrdersMap.empty()) {
        auto bestBuy = buyOrdersMap.rbegin();
        auto bestSell = sellOrdersMap.begin();

        if (bestBuy->first >= bestSell->first) {
            auto& buyQueue = bestBuy->second;
            auto& sellQueue = bestSell->second;

            Order& currentBuy = buyQueue.front();
            Order& currentSell = sellQueue.front();

            int matchedQuantity = (currentBuy.quantity < currentSell.quantity) ? currentBuy.quantity : currentSell.quantity;

            std::string message = "Orders Matched: " + std::to_string(matchedQuantity) + " @ " + std::to_string(bestSell->first);
            broadcastToClients(message);

            currentBuy.quantity -= matchedQuantity;
            currentSell.quantity -= matchedQuantity;

            if (currentBuy.quantity == 0) {
                buyQueue.pop();
                if (buyQueue.empty()) {
                    buyOrdersMap.erase(bestBuy->first);
                }
            }
            if (currentSell.quantity == 0) {
                sellQueue.pop();
                if (sellQueue.empty()) {
                    sellOrdersMap.erase(bestSell->first);
                }
            }
        } else {
            break;
        }
    }
}

void OrderBook::printOrderBook() const {
    std::lock_guard<std::mutex> buyLock(buyOrdersMutex);
    std::lock_guard<std::mutex> sellLock(sellOrdersMutex);

    std::cout << "Buy Orders: \n";
    for (const auto& [price, orders] : buyOrdersMap) {
        std::cout << "Price: " << price << ", Quantity: " << orders.front().quantity << "\n";
    }

    std::cout << "Sell Orders:\n";
    for (const auto& [price, orders] : sellOrdersMap) {
        std::cout << "Price: " << price << ", Quantity: " << orders.front().quantity << "\n";
    }
}

void OrderBook::waitForAllOrders() {
    for (auto& future : matchOrdersFutures) {
        if (future.valid()) {
            future.get();
        }
    }
    matchOrdersFutures.clear();
}

void OrderBook::broadcastToClients(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (SOCKET clientSocket : clients) {
        send(clientSocket, message.c_str(), message.size(), 0);
    }
}

void OrderBook::addClient(SOCKET clientSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.push_back(clientSocket);
}

void OrderBook::removeClient(SOCKET clientSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto it = std::remove(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }
}