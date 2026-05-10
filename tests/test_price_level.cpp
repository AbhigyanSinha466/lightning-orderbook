#include <catch2/catch_test_macros.hpp>
#include "price_level.hpp"

using namespace engine;

TEST_CASE("PriceLevel operations", "[price_level]") {
    PriceLevel level;
    CHECK(level.is_empty());

    Order o1(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000);
    Order o2(2, "AAPL", Side::Buy, OrderType::Limit, 100, 20, 1001);
    Order o3(3, "AAPL", Side::Buy, OrderType::Limit, 100, 30, 1002);

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
