#pragma once

#include "order.hpp"
#include <list>

namespace engine {

/**
 * @brief PriceLevel represents all resting orders at a single price point on one side of the book.
 * It does not own the Order objects.
 */
class PriceLevel {
public:
    /**
     * @brief Adds an order to the end of the level (time priority).
     * Sets the order's level_it to its position in the list.
     * @param o Pointer to the order to add.
     */
    void add_order(Order* o);

    /**
     * @brief Removes an order from the level using its cached iterator.
     * @param o Pointer to the order to remove.
     */
    void remove_order(Order* o);

    /**
     * @brief Returns the sum of quantities of all orders in this level.
     * O(n) operation.
     */
    Qty total_quantity() const;

    /**
     * @brief Checks if there are no orders in this level.
     */
    bool is_empty() const;

    /**
     * @brief Returns the first (oldest) order in the level.
     */
    Order* front() const;

    /**
     * @brief Removes the first order from the level.
     */
    void pop_front();

private:
    std::list<Order*> orders;
};

} // namespace engine
