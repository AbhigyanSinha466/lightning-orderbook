#include "order_book.hpp"

namespace engine {

void OrderBook::add_order(Order* o) {
    if (o->side == Side::Buy) {
        bids[o->price].add_order(o);
    } else {
        asks[o->price].add_order(o);
    }
}

void OrderBook::remove_order(Order* o) {
    if (o->side == Side::Buy) {
        auto it = bids.find(o->price);
        if (it != bids.end()) {
            it->second.remove_order(o);
            if (it->second.is_empty()) {
                bids.erase(it);
            }
        }
    } else {
        auto it = asks.find(o->price);
        if (it != asks.end()) {
            it->second.remove_order(o);
            if (it->second.is_empty()) {
                asks.erase(it);
            }
        }
    }
}

PriceLevel* OrderBook::best_bid() {
    if (bids.empty()) return nullptr;
    return &bids.begin()->second;
}

PriceLevel* OrderBook::best_ask() {
    if (asks.empty()) return nullptr;
    return &asks.begin()->second;
}

Price OrderBook::best_bid_price() const {
    if (bids.empty()) return std::numeric_limits<Price>::min();
    return bids.begin()->first;
}

Price OrderBook::best_ask_price() const {
    if (asks.empty()) return std::numeric_limits<Price>::max();
    return asks.begin()->first;
}

Price OrderBook::spread() const {
    if (bids.empty() || asks.empty()) return 0;
    return best_ask_price() - best_bid_price();
}

} // namespace engine
