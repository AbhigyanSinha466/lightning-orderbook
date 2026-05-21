#pragma once

#include "types.hpp"
#include <list>

namespace engine {

/**
 * @brief Order represents a single order in the system.
 * It is a plain data struct used as a container.
 */
struct alignas(64) Order {
    OrderId id;
    Symbol symbol;
    Side side;
    OrderType type;
    Price price;
    Qty quantity;
    Timestamp timestamp;

    /**
     * @brief Pointers for intrusive doubly-linked list in PriceLevel.
     */
    Order* next = nullptr;
    Order* prev = nullptr;

    Order(OrderId id, Symbol symbol, Side side, OrderType type, Price price, Qty quantity, Timestamp timestamp)
        : id(id), symbol(symbol), side(side), type(type), price(price), quantity(quantity), timestamp(timestamp) {}
};

} // namespace engine
