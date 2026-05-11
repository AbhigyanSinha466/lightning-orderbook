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
    PriceLevel() : head(nullptr), tail(nullptr) {}

    void add_order(Order* o);
    void remove_order(Order* o);
    Qty total_quantity() const;
    bool is_empty() const { return head == nullptr; }
    Order* front() const { return head; }
    void pop_front();

private:
    Order* head;
    Order* tail;
};

} // namespace engine
