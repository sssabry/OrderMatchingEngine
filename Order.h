#ifndef ORDER_H
#define ORDER_H

enum class OrderType { Market, Limit };
enum class Side { Buy, Sell };

struct Order {
    int id;
    OrderType type;
    Side side;
    double price;
    int quantity;

    void print() const;
};

#endif // ORDER_H
