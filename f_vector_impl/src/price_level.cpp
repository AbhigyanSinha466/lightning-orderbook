#include "price_level.hpp"

namespace engine {

void PriceLevel::add_order(Order* o) {
    o->next = nullptr;
    o->prev = tail;
    if (tail) {
        tail->next = o;
    } else {
        head = o;
    }
    tail = o;
    cumulative_qty += o->quantity;
}

void PriceLevel::remove_order(Order* o) {
    if (o->prev) {
        o->prev->next = o->next;
    } else {
        head = o->next;
    }
    
    if (o->next) {
        o->next->prev = o->prev;
    } else {
        tail = o->prev;
    }
    cumulative_qty -= o->quantity;
    o->next = nullptr;
    o->prev = nullptr;
}

void PriceLevel::pop_front() {
    if (head) {
        remove_order(head);
    }
}

} // namespace engine
