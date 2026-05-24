#pragma once

#define IS_F_VECTOR_IMPL 1

#include "order_book.hpp"
#include <memory>
#include <functional>
#include "absl/container/flat_hash_map.h"
#include "utils/order_pool.hpp"

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
     * @param o Order object by value.
     */
    void submit_order(Order o);

    /**
     * @brief Cancels an existing order by its ID.
     */
    void cancel_order(OrderId id);

    /**
     * @brief Reduces the quantity of an existing order.
     */
    void reduce_order(OrderId id, Qty cancelled_qty);

    /**
     * @brief Replaces an existing order with a new one.
     */
    void replace_order(OrderId old_id, OrderId new_id, Price new_price, Qty new_qty, Timestamp new_ts);

private:
    /**
     * @brief Internal matching logic.
     */
    void match(Order* aggressive, OrderBook& book);

    absl::flat_hash_map<Symbol, OrderBook> books;
    absl::flat_hash_map<OrderId, Order*> order_store;
    
    // Custom memory pool for orders
    utils::OrderPool order_pool;
    
    FillCallback on_fill;
};

} // namespace engine
