#include "price_level.hpp"
#include <numeric>

namespace engine {

void PriceLevel::add_order(Order* o) {
    orders.push_back(o);
    o->level_it = std::prev(orders.end());
}

void PriceLevel::remove_order(Order* o) {
    orders.erase(o->level_it);
}

Qty PriceLevel::total_quantity() const {
    return std::accumulate(orders.begin(), orders.end(), 0u,
        [](Qty sum, const Order* o) { return sum + o->quantity; });
}

bool PriceLevel::is_empty() const {
    return orders.empty();
}

Order* PriceLevel::front() const {
    if (orders.empty()) return nullptr;
    return orders.front();
}

void PriceLevel::pop_front() {
    if (!orders.empty()) {
        orders.pop_front();
    }
}

} // namespace engine
