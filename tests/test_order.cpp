#include <catch2/catch_test_macros.hpp>
#include "order.hpp"

using namespace engine;

namespace {
Symbol to_symbol(const std::string& s) {
    std::string padded = s;
    padded.resize(8, ' ');
    uint64_t hash = 0;
    for (int i = 0; i < 8; ++i) {
        hash = hash * 31 + static_cast<uint8_t>(padded[i]);
    }
    return hash;
}
}

TEST_CASE("Order construction", "[order]") {
    OrderId id = 12345;
    Symbol symbol = to_symbol("AAPL");
    Side side = Side::Buy;
    OrderType type = OrderType::Limit;
    Price price = 150 * PRICE_SCALE;
    Qty qty = 100;
    Timestamp ts = 987654321;

    Order order(id, symbol, side, type, price, qty, ts);

    CHECK(order.id == id);
    CHECK(order.symbol == symbol);
    CHECK(order.side == side);
    CHECK(order.type == type);
    CHECK(order.price == price);
    CHECK(order.quantity == qty);
    CHECK(order.timestamp == ts);
}
