#include <iostream> 
#include <map> 
#include <queue> 

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
    private:
        // Queues of orders at each price:
        std::map<double, std::queue<Order> > buyOrdersMap; 
        std::map<double, std::queue<Order> > sellOrdersMap;
};

void OrderBook::addOrder(const Order& order) { // passes reference of an order object
    if (order.side == Side::Buy) {
        buyOrdersMap[order.price].push(order); // pushes it onto queue, mapped by the price of the order
    } else {
        sellOrdersMap[order.price].push(order);
    }

    matchOrders();
}

void OrderBook::matchOrders() {
    while (!buyOrdersMap.empty() && !sellOrdersMap.empty()) { // while neither queue-map is empty
        auto bestBuy = buyOrdersMap.rbegin(); // reverse iteration, get the lastt element (AKA highest price)
        auto bestSell = sellOrdersMap.begin(); // lowest selling price, first element

        if (bestBuy->first >= bestSell->first) { // getting first component of pair AKA the price
            // Getting the second component -> the order queues
            auto& buyQueue = bestBuy->second;
            auto& sellQueue = bestSell->second;

            // Temp vars holding the current front of each queue:
            Order& currentBuy = buyQueue.front();
            Order& currentSell = sellQueue.front();

            // Extract minimum overlapping quantity:
            int matchedQuantity = std::min(currentBuy.quantity, currentSell.quantity);

            // Print buy statement @ buyOrder's price (highest) & minimum overlapped quantity:
            std::cout << "Orders Matched: " << matchedQuantity << " @ " << bestSell->first << "\n";

            currentBuy.quantity -= matchedQuantity;
            currentSell.quantity -= matchedQuantity;

            // Removing if empty to avoid breaking:
            if (currentBuy.quantity == 0) {
                buyQueue.pop();
                if(buyQueue.empty()) {
                    buyOrdersMap.erase(bestBuy->first); // if empty, delete this element (price point) from map
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
    std::cout << "Buy Orders: \n";
    for (const auto& [price, orders] : buyOrdersMap) { // for each price & orders in buyOrdersMap 
        std::cout << "Price: " << price << ", Quantity: " << orders.front().quantity << "\n";
    }

    std::cout << "Sell Orders:\n";
    for (const auto& [price, orders] : sellOrdersMap) {
        std::cout << "Price: " << price << ", Quantity: " << orders.front().quantity << "\n";
    }
}

// Current (temporary) way of interacting with the engine -- FIXME
int main() {
    OrderBook orderBook;

    std::cout << "Adding orders...\n";
    orderBook.addOrder({1, OrderType::Limit, Side::Buy, 100.5, 10}); // Buy 10 @ 100.5
    orderBook.addOrder({2, OrderType::Limit, Side::Sell, 100.5, 5});  // Sell 5 @ 100.5
    orderBook.addOrder({3, OrderType::Limit, Side::Buy, 101.0, 15}); // Buy 15 @ 101.0
    orderBook.addOrder({4, OrderType::Limit, Side::Sell, 100.0, 10}); // Sell 10 @ 100.0

    std::cout << "\nOrder book after adding orders:\n";
    orderBook.printOrderBook();

    std::cout << "\nAdding more orders...\n";
    orderBook.addOrder({5, OrderType::Limit, Side::Sell, 101.0, 10}); // Sell 10 @ 101.0
    orderBook.addOrder({6, OrderType::Limit, Side::Buy, 102.0, 20}); // Buy 20 @ 102.0

    std::cout << "\nOrder book after additional orders:\n";
    orderBook.printOrderBook();

    return 0;
}

