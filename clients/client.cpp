#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

void startClient() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << "\n";
        return;
    }

    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed, error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(60000);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(connectSocket, (sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed, error: " << WSAGetLastError() << "\n";
        closesocket(connectSocket);
        WSACleanup();
        return;
    }

    char buffer[256];
    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(connectSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Connection closed or error occurred.\n";
            break;
        }
        std::cout << "Server: " << buffer;
    }

    closesocket(connectSocket);
    WSACleanup();
}

int main() {
    startClient();
    return 0;
}
