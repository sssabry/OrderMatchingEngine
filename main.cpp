#include <iostream>
#include <thread>
#include <cstdlib>
#include <ctime>
#include "OrderBook.h"
#include "Server.h"

Order generateRandomOrder(int id) {
    Order order;
    order.id = id;
    order.type = OrderType::Limit;
    order.side = (rand() % 2 == 0) ? Side::Buy : Side::Sell;
    order.price = 100.0 + static_cast<double>(rand() % 2000) / 10.0;
    order.quantity = rand() % 100 + 1;
    return order;
}

void generateOrdersPeriodically(OrderBook& orderBook, int& nextOrderId) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(15));
        Order randomOrder = generateRandomOrder(nextOrderId++);
        orderBook.addOrder(randomOrder);
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    OrderBook orderBook;
    int nextOrderId = 1; 

    std::thread orderGenerator(generateOrdersPeriodically, std::ref(orderBook), std::ref(nextOrderId));
    orderGenerator.detach();

    startServer(orderBook);

    return 0;
}
