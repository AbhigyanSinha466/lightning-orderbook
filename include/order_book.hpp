#pragma once

#include "price_level.hpp"
#include <map>
#include <functional>
#include <limits>

namespace engine {

/**
 * @brief OrderBook represents the two-sided book for a single instrument.
 * It manages PriceLevels and ensures the best bid/ask are accessible in O(1).
 */
class OrderBook {
public:
    /**
     * @brief Adds a limit order to the book.
     */
    void add_order(Order* o);

    /**
     * @brief Removes an order from the book.
     * Erases the price level if it becomes empty.
     */
    void remove_order(Order* o);

    /**
     * @brief Returns the best bid PriceLevel.
     */
    PriceLevel* best_bid();

    /**
     * @brief Returns the best ask PriceLevel.
     */
    PriceLevel* best_ask();

    /**
     * @brief Returns the best bid price.
     */
    Price best_bid_price() const;

    /**
     * @brief Returns the best ask price.
     */
    Price best_ask_price() const;

    /**
     * @brief Returns the spread (best_ask - best_bid).
     */
    Price spread() const;

private:
    std::map<Price, PriceLevel, std::greater<Price>> bids;
    std::map<Price, PriceLevel> asks;
};

} // namespace engine
