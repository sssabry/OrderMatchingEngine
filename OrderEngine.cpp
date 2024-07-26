#include <iostream>
#include <map>
#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

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
    matchOrders();
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
            int matchedQuantity = (currentBuy.quantity < currentSell.quantity) ? currentBuy.quantity : currentSell.quantity;

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

Order generateRandomOrder(int id) {
    Order order;
    order.id = id;
    order.type = OrderType::Limit;
    order.side = (rand() % 2 == 0) ? Side::Buy : Side::Sell;
    order.price = 100.0 + static_cast<double>(rand() % 2000) /10.0;
    order.quantity = rand() % 100 + 1;
    return order;
}
/*
// Terminal-based interactivity with the order engine -- OLD
void placeOrder(OrderBook& orderBook, int& nextOrderId) {
    Order order;
    order.id = nextOrderId++;
    order.type = OrderType::Limit;

    int side;
    std::cout << "Enter side (0 for Buy, 1 for Sell): ";
    std::cin >> side;
    order.side = (side == 0) ? Side::Buy : Side::Sell;

    std::cout << "Enter price: ";
    std::cin >> order.price;

    std::cout << "Enter quantity: ";
    std::cin >> order.quantity;

    orderBook.addOrder(order);
}
*/
void generateOrdersPeriodically(OrderBook& orderBook, int& nextOrderId) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(15));
        Order randomOrder = generateRandomOrder(nextOrderId++);
        orderBook.addOrder(randomOrder);
        std::cout << "\n Random Order Added: "
                  << (randomOrder.side == Side::Buy ? "Buy" : "Sell")
                  << " " << randomOrder.quantity << " @ " << randomOrder.price << "\n";
    }
}

// CHange to act like a server:
void handleClient(OrderBook& orderBook, SOCKET clientSocket) {
    int nextOrderId = 1;
    char buffer[256];
    while (true) {
        std::memset(buffer, 0, 256);
        int bytesReceived = recv(clientSocket, buffer, 255, 0);
        if (bytesReceived <= 0) {
            std::cout << "Client disconnected or error occurred.\n";
            break;
        }

        int side;
        double price;
        int quantity;
        sscanf(buffer, "%d %lf %d", &side, &price, &quantity);

        Order order = { nextOrderId++, OrderType::Limit, side == 0 ? Side::Buy : Side::Sell, price, quantity };
        orderBook.addOrder(order);

        std::string confirmation = "Order added: " + std::string(buffer) + "\n";
        send(clientSocket, confirmation.c_str(), confirmation.size(), 0);
    }
    closesocket(clientSocket);
}

void startServer(OrderBook& orderBook) {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << "\n";
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Cannot create socket, error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(60000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed, error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed, error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Server started. Waiting for connections...\n";

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed, error: " << WSAGetLastError() << "\n";
            continue;
        }

        std::cout << "Client connected.\n";
        std::thread(handleClient, std::ref(orderBook), clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    OrderBook orderBook;
    int nextOrderId = 1;

    std::thread orderGenerator(generateOrdersPeriodically, std::ref(orderBook), std::ref(nextOrderId));
    std::thread serverThread(startServer, std::ref(orderBook));

    orderGenerator.join();
    serverThread.join();

    return 0;
}

/*
// Current (temporary) way of interacting with the engine -- FIXME
int main() {
    srand(static_cast<unsigned int>(time(0)));
    OrderBook orderBook;
    int nextOrderId = 1;

    // Start background thread to generate random orders
    std::thread orderGenerator(generateOrdersPeriodically, std::ref(orderBook), std::ref(nextOrderId));

    int choice;
    do {
        std::cout << "\nMenu:\n";
        std::cout << "1. Place Order\n";
        std::cout << "2. Print Order Book\n";
        std::cout << "3. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                placeOrder(orderBook, nextOrderId);
                break;
            case 2:
                orderBook.waitForAllOrders();
                orderBook.printOrderBook();
                break;
            case 3:
                std::cout << "Exiting...\n";
                orderBook.waitForAllOrders();
                break;
            default:
                std::cout << "Invalid choice, please try again.\n";
                break;
        }
    } while (choice != 3);

    // Signal the order generator to stop (this is a simplified approach)
    // For real applications, you should use proper shutdown mechanisms
    orderGenerator.detach();

    return 0;
}
*/