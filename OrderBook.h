#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <queue>
#include <mutex>
#include <future>
#include <vector>
#include <winsock2.h>  // For SOCKET

#include "Order.h"

class OrderBook {
public:
    void addOrder(const Order& order);
    void matchOrders();
    void printOrderBook() const;
    void waitForAllOrders();
    void broadcastToClients(const std::string& message);
    void addClient(SOCKET clientSocket);
    void removeClient(SOCKET clientSocket);

private:
    std::map<double, std::queue<Order>> buyOrdersMap;
    std::map<double, std::queue<Order>> sellOrdersMap;
    mutable std::mutex buyOrdersMutex;
    mutable std::mutex sellOrdersMutex;
    std::mutex clientsMutex;
    std::vector<SOCKET> clients; 
    std::vector<std::future<void>> matchOrdersFutures;
};

#endif // ORDERBOOK_H
