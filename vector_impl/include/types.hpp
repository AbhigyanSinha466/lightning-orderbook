#pragma once

#include <cstdint>
#include <string>
#include <array>

namespace engine {

/**
 * @brief Price is represented as a fixed-point integer with 4 decimal places per ITCH 5.0.
 * A price of $100.00 is represented as 1,000,000 (100 * 10,000).
 */
using Price = int64_t;

/**
 * @brief Quantity of an order. Non-negative.
 */
using Qty = uint32_t;

/**
 * @brief Unique order reference number.
 */
using OrderId = uint64_t;

/**
 * @brief Instrument symbol. Hashed as uint64_t for O(1) comparison and lookup.
 */
using Symbol = uint64_t;

/**
 * @brief Nanoseconds since midnight.
 */
using Timestamp = uint64_t;

/**
 * @brief Side of the order book.
 */
enum class Side : uint8_t {
    Buy,
    Sell
};

/**
 * @brief Type of the order.
 */
enum class OrderType : uint8_t {
    Limit,
    Market
};

/**
 * @brief Scale factor for price to convert to integer representation (ITCH 5.0).
 */
constexpr Price PRICE_SCALE = 10000;

/**
 * @brief FillEvent represents a trade execution.
 */
struct FillEvent {
    OrderId aggressive_id;
    OrderId passive_id;
    Price fill_price;
    Qty fill_qty;
    Timestamp timestamp;
};

} // namespace engine
