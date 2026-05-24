#include <catch2/catch_test_macros.hpp>
#include "price_level.hpp"

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

TEST_CASE("PriceLevel operations", "[price_level]") {
    PriceLevel level;
    CHECK(level.is_empty());

    Symbol symbol = to_symbol("AAPL");
    Order o1(1, symbol, Side::Buy, OrderType::Limit, 100, 10, 1000);
    Order o2(2, symbol, Side::Buy, OrderType::Limit, 100, 20, 1001);
    Order o3(3, symbol, Side::Buy, OrderType::Limit, 100, 30, 1002);

    SECTION("Add orders") {
        level.add_order(&o1);
        level.add_order(&o2);
        CHECK_FALSE(level.is_empty());
        CHECK(level.total_quantity() == 30);
        CHECK(level.front() == &o1);
    }

    SECTION("Remove middle order") {
        level.add_order(&o1);
        level.add_order(&o2);
        level.add_order(&o3);
        level.remove_order(&o2);
        CHECK(level.total_quantity() == 40);
        CHECK(level.front() == &o1);
        level.pop_front();
        CHECK(level.front() == &o3);
    }

    SECTION("Emptying level") {
        level.add_order(&o1);
        level.pop_front();
        CHECK(level.is_empty());
        CHECK(level.front() == nullptr);
    }
}
