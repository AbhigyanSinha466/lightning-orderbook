#pragma once

#include "order.hpp"
#include <list>

namespace engine {

/**
 * @brief PriceLevel represents all resting orders at a single price point on one side of the book.
 * It does not own the Order objects.
 */
class alignas(64) PriceLevel {
public:
    PriceLevel() : head(nullptr), tail(nullptr) {}

    /**
     * @brief Adds an order to the end of the level (time priority).
     */
    void add_order(Order* o);

    /**
     * @brief Removes an order from the level.
     */
    void remove_order(Order* o);

    /**
     * @brief Returns the sum of quantities of all orders in this level.
     * O(1) operation (cached).
     */
    Qty total_quantity() const { return cumulative_qty; }

    /**
     * @brief Decrements the cumulative quantity of the level.
     * Used when an order at this level is partially or fully filled.
     */
    void decrement_cumulative_qty(Qty qty) { cumulative_qty -= qty; }

    /**
     * @brief Checks if there are no orders in this level.
     */
    bool is_empty() const { return head == nullptr; }

    /**
     * @brief Returns the first (oldest) order in the level.
     */
    Order* front() const { return head; }

    /**
     * @brief Removes the first order from the level.
     */
    void pop_front();

private:
    Order* head;
    Order* tail;
    Qty cumulative_qty = 0;
};

} // namespace engine
