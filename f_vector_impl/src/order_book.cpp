#include "order_book.hpp"

namespace engine {

PriceLevel& OrderBook::get_or_create_level(std::vector<PriceEntry>& side, Price price, bool is_bid) {
    if (!side.empty()) {
        if (side.back().price == price) {
            return side.back().level;
        }
        
        bool new_best = is_bid ? (price > side.back().price) : (price < side.back().price);
        if (new_best) {
            side.emplace_back(price);
            return side.back().level;
        }
    } else {
        side.emplace_back(price);
        return side.back().level;
    }

    // Linear search from best price (back) to worst price (front)
    // Bids ascending (98, 99, 100), Asks descending (103, 102, 101)
    auto comp = [is_bid](Price p1, Price p2) {
        return is_bid ? (p1 < p2) : (p1 > p2);
    };

    auto it = side.end();
    while (it != side.begin()) {
        --it;
        if (it->price == price) return it->level;
        if (comp(it->price, price)) {
            // Since side is sorted such that back() is the "best" price,
            // if we find an entry where it->price is "worse" than price,
            // we've passed the insertion point.
            return side.emplace(it + 1, price)->level;
        }
    }

    // If we reach here, the new price is the "worst" price and should be at the front
    return side.emplace(side.begin(), price)->level;
}

void OrderBook::add_order(Order* o) {
    if (o->side == Side::Buy) {
        get_or_create_level(bids, o->price, true).add_order(o);
    } else {
        get_or_create_level(asks, o->price, false).add_order(o);
    }
}

void OrderBook::remove_order(Order* o) {
    auto& side = (o->side == Side::Buy) ? bids : asks;
    bool is_bid = (o->side == Side::Buy);

    if (side.empty()) return;

    if (side.back().price == o->price) {
        side.back().level.remove_order(o);
        if (side.back().level.is_empty()) {
            side.pop_back();
        }
        return;
    }

    auto comp = [is_bid](Price p1, Price p2) {
        return is_bid ? (p1 < p2) : (p1 > p2);
    };

    auto it = side.end();
    while (it != side.begin()) {
        --it;
        if (it->price == o->price) {
            it->level.remove_order(o);
            if (it->level.is_empty()) {
                side.erase(it);
            }
            return;
        }
        if (comp(it->price, o->price)) {
            // Price not found in the book
            break;
        }
    }
}

PriceLevel* OrderBook::best_bid() {
    return bids.empty() ? nullptr : &bids.back().level;
}

PriceLevel* OrderBook::best_ask() {
    return asks.empty() ? nullptr : &asks.back().level;
}

Price OrderBook::best_bid_price() const {
    return bids.empty() ? std::numeric_limits<Price>::min() : bids.back().price;
}

Price OrderBook::best_ask_price() const {
    return asks.empty() ? std::numeric_limits<Price>::max() : asks.back().price;
}

Price OrderBook::spread() const {
    if (bids.empty() || asks.empty()) return 0;
    return best_ask_price() - best_bid_price();
}

} // namespace engine
