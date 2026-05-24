#pragma once

#include "order_book.hpp"
#include <unordered_map>
#include <memory>
#include <functional>

namespace engine {

/**
 * @brief MatchingEngine is the top-level object of the Engine Core.
 * It owns all orders and drives the matching logic for multiple symbols.
 */
class MatchingEngine {
public:
    using FillCallback = std::function<void(const FillEvent&)>;

    /**
     * @brief Sets the callback for fill events.
     */
    void set_on_fill(FillCallback cb);

    /**
     * @brief Submits a new order to the engine.
     * Takes ownership of the order if it rests on the book.
     * @param o Pointer to the order (newly allocated).
     */
    void submit_order(std::unique_ptr<Order> o);

    /**
     * @brief Cancels an existing order by its ID.
     */
    void cancel_order(OrderId id);

    /**
     * @brief Reduces the quantity of an existing order.
     * If the quantity reaches zero, the order is removed.
     */
    void reduce_order(OrderId id, Qty cancelled_qty);

    /**
     * @brief Replaces an existing order with new price/quantity and resets time priority.
     */
    void replace_order(OrderId old_id, OrderId new_id, Price new_price, Qty new_qty, Timestamp new_ts);

private:
    /**
     * @brief Internal matching loop.
     * @param aggressive The incoming order.
     * @param book The order book for the aggressive order's symbol.
     */
    void match(Order* aggressive, OrderBook& book);

    std::unordered_map<OrderId, std::unique_ptr<Order>> order_store;
    std::unordered_map<Symbol, OrderBook> books;
    FillCallback on_fill;
};

} // namespace engine
