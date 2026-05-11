#pragma once

#include "types.hpp"
#include <list>

namespace engine {

/**
 * @brief Order represents a single order in the system.
 * It is a plain data struct used as a container.
 */
struct Order {
    OrderId id;
    Symbol symbol;
    Side side;
    OrderType type;
    Price price;
    Qty quantity;
    Timestamp timestamp;

    /**
     * @brief Iterator to the order's position in a PriceLevel's list.
     * Cached to allow O(1) removal from the PriceLevel.
     */
    std::list<Order*>::iterator level_it;

    Order(OrderId id, Symbol symbol, Side side, OrderType type, Price price, Qty quantity, Timestamp timestamp)
        : id(id), symbol(std::move(symbol)), side(side), type(type), price(price), quantity(quantity), timestamp(timestamp) {}
};

} // namespace engine
