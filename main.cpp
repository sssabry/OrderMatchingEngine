#include <iostream> 
#include <map> 
#include <queue> 
#include <mutex>
#include <future>
#include <thread>
#include <vector> 

enum class OrderType {Market, Limit}; 
enum class Side {Buy, Sell};

struct Order {
    int id;
    OrderType type;
    Side side;
    double price;
    int quantity;
};

class OrderBook {
public:
    void addOrder(const Order& order);
    void matchOrders();
    void printOrderBook() const;
    void waitForAllOrders(); 
    
private:

    // Queues of orders at each price:
    std::map<double, std::queue<Order>> buyOrdersMap; 
    std::map<double, std::queue<Order>> sellOrdersMap;
    // Thread safety:
    mutable std::mutex buyOrdersMutex;
    mutable std::mutex sellOrdersMutex;
    // Holder for async tasks:
    std::vector<std::future<void>> matchOrdersFutures; 
};

void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(order.side == Side::Buy ? buyOrdersMutex : sellOrdersMutex);

    if (order.side == Side::Buy) {
        buyOrdersMap[order.price].push(order);
    } else {
        sellOrdersMap[order.price].push(order);
    }

    // Parallel call to matchOrders: (non-blocking execution)
    matchOrdersFutures.push_back(std::async(std::launch::async, [this]() { this->matchOrders(); }));
}

void OrderBook::matchOrders() {
    // Temporary lock, freeze the book:
    std::lock_guard<std::mutex> buyLock(buyOrdersMutex);
    std::lock_guard<std::mutex> sellLock(sellOrdersMutex);

    while (!buyOrdersMap.empty() && !sellOrdersMap.empty()) {
        auto bestBuy = buyOrdersMap.rbegin(); // highest buy price (get last element using reverse begin iteration)
        auto bestSell = sellOrdersMap.begin(); // lowest sell price

        if (bestBuy->first >= bestSell->first) { // get first compoent of pair (AKA price)

            // Second component -> the queues at this price
            auto& buyQueue = bestBuy->second;
            auto& sellQueue = bestSell->second;

            // Temp holds of current fronts of each queue:
            Order& currentBuy = buyQueue.front();
            Order& currentSell = sellQueue.front();

            // Minimum overlapping quantity:
            int matchedQuantity = std::min(currentBuy.quantity, currentSell.quantity);

            std::cout << "Orders Matched: " << matchedQuantity << " @ " << bestSell->first << "\n";

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
    // Freeze book:
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
    for (auto& future : matchOrdersFutures) { // for each future
        if (future.valid()) {
            future.get();
        }
    }
    matchOrdersFutures.clear(); 
}

// Current (temporary) way of interacting with the engine -- FIXME
int main() {
    OrderBook orderBook;

    std::cout << "Adding orders...\n";
    orderBook.addOrder({1, OrderType::Limit, Side::Buy, 100.5, 10}); // Buy 10 @ 100.5
    orderBook.addOrder({2, OrderType::Limit, Side::Sell, 100.5, 5});  // Sell 5 @ 100.5
    orderBook.addOrder({3, OrderType::Limit, Side::Buy, 101.0, 15}); // Buy 15 @ 101.0
    orderBook.addOrder({4, OrderType::Limit, Side::Sell, 100.0, 10}); // Sell 10 @ 100.0

    orderBook.waitForAllOrders();

    std::cout << "\nOrder book after adding orders:\n";
    orderBook.printOrderBook();

    std::cout << "\nAdding more orders...\n";
    orderBook.addOrder({5, OrderType::Limit, Side::Sell, 101.0, 10}); // Sell 10 @ 101.0
    orderBook.addOrder({6, OrderType::Limit, Side::Buy, 102.0, 20}); // Buy 20 @ 102.0

    orderBook.waitForAllOrders();

    std::cout << "\nOrder book after additional orders:\n";
    orderBook.printOrderBook();

    return 0;
}
