#include "OrderBook.h"
#include <iostream>
#include <thread>
#include <cstring>
#include <winsock2.h>  // For SOCKET

void handleClient(OrderBook& orderBook, SOCKET clientSocket) {
    int nextOrderId = 1;
    char buffer[256];
    while (true) {
        std::memset(buffer, 0, 256);
        int bytesReceived = recv(clientSocket, buffer, 255, 0);
        if (bytesReceived <= 0) {
            std::cout << "Client disconnected or error occurred.\n";
            orderBook.removeClient(clientSocket); // Remove client when disconnected
            break;
        }

        int side;
        double price;
        int quantity;
        sscanf_s(buffer, "%d %lf %d", &side, &price, &quantity);

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
        orderBook.addClient(clientSocket); 
        std::thread(handleClient, std::ref(orderBook), clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
}
