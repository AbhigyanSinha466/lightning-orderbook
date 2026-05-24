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
    o->next = nullptr;
    o->prev = nullptr;
}

Qty PriceLevel::total_quantity() const {
    Qty sum = 0;
    Order* curr = head;
    while (curr) {
        sum += curr->quantity;
        curr = curr->next;
    }
    return sum;
}

void PriceLevel::pop_front() {
    if (head) {
        remove_order(head);
    }
}

} // namespace engine
