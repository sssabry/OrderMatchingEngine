#ifndef SERVER_H
#define SERVER_H

#include "OrderBook.h"
#include <winsock2.h>

void handleClient(OrderBook& orderBook, SOCKET clientSocket);
void startServer(OrderBook& orderBook);

#endif // SERVER_H
