#pragma once

#include "price_level.hpp"
#include <vector>
#include <algorithm>
#include <limits>

namespace engine {

/**
 * @brief PriceEntry pairs a Price with its PriceLevel for the vector-based book.
 */
struct PriceEntry {
    Price price;
    PriceLevel level;

    PriceEntry(Price p) : price(p) {}
};

/**
 * @brief OrderBook refactored to use std::vector for better cache locality.
 * It maintains sorted vectors of PriceEntry to simulate a map (Flat Map).
 */
class OrderBook {
public:
    OrderBook() {
        bids.reserve(128);
        asks.reserve(128);
    }

    void add_order(Order* o);
    void remove_order(Order* o);

    PriceLevel* best_bid();
    PriceLevel* best_ask();

    Price best_bid_price() const;
    Price best_ask_price() const;

    Price spread() const;

private:
    // Bids sorted descending (highest price first)
    std::vector<PriceEntry> bids;
    // Asks sorted ascending (lowest price first)
    std::vector<PriceEntry> asks;

    // Helper to find or create a price level
    PriceLevel& get_or_create_level(std::vector<PriceEntry>& side, Price price, bool descending);
};

} // namespace engine
