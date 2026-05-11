#include "order_book.hpp"

namespace engine {

PriceLevel& OrderBook::get_or_create_level(std::vector<PriceEntry>& side, Price price, bool is_bid) {
    if (!side.empty()) {
        if (side.back().price == price) [[likely]] {
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

    // Branchless binary search fallback (from CppCon 2024 presentation)
    // Bids ascending (98, 99, 100), Asks descending (103, 102, 101)
    auto comp = [is_bid](Price p1, Price p2) {
        return is_bid ? (p1 < p2) : (p1 > p2);
    };

    auto first = side.begin();
    auto length = std::distance(first, side.end());
    while (length > 0) {
        auto half = length / 2;
        auto mid = first + half;
        if (mid->price == price) return mid->level;
        
        // Use the branchless step: first += (mid < target) ? (length - half) : 0
        if (comp(mid->price, price)) {
            first = mid + 1;
            length = length - half - 1;
        } else {
            length = half;
        }
    }

    // Insert at the found position
    return side.emplace(first, price)->level;
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

    if (!side.empty() && side.back().price == o->price) {
        side.back().level.remove_order(o);
        if (side.back().level.is_empty()) {
            side.pop_back();
        }
        return;
    }

    auto comp = [is_bid](Price p1, Price p2) {
        return is_bid ? (p1 < p2) : (p1 > p2);
    };

    auto first = side.begin();
    auto length = std::distance(first, side.end());
    while (length > 0) {
        auto half = length / 2;
        auto mid = first + half;
        if (mid->price == o->price) {
            mid->level.remove_order(o);
            if (mid->level.is_empty()) {
                side.erase(mid);
            }
            return;
        }
        
        if (comp(mid->price, o->price)) {
            first = mid + 1;
            length = length - half - 1;
        } else {
            length = half;
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
