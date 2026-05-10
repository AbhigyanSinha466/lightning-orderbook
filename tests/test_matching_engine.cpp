#include <catch2/catch_test_macros.hpp>
#include "matching_engine.hpp"
#include <vector>

using namespace engine;

TEST_CASE("MatchingEngine scenarios", "[matching_engine]") {
    MatchingEngine engine;
    std::vector<FillEvent> fills;
    engine.set_on_fill([&](const FillEvent& e) { fills.push_back(e); });

    SECTION("Scenario 1: No match") {
        engine.submit_order(std::make_unique<Order>(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000));
        engine.submit_order(std::make_unique<Order>(2, "AAPL", Side::Sell, OrderType::Limit, 101, 10, 1001));
        CHECK(fills.empty());
    }

    SECTION("Scenario 2: Exact match") {
        engine.submit_order(std::make_unique<Order>(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000));
        engine.submit_order(std::make_unique<Order>(2, "AAPL", Side::Sell, OrderType::Limit, 100, 10, 1001));
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].aggressive_id == 2);
        CHECK(fills[0].passive_id == 1);
        CHECK(fills[0].fill_qty == 10);
        CHECK(fills[0].fill_price == 100);
    }

    SECTION("Scenario 3: Partial fill") {
        engine.submit_order(std::make_unique<Order>(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000));
        engine.submit_order(std::make_unique<Order>(2, "AAPL", Side::Sell, OrderType::Limit, 100, 6, 1001));
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].fill_qty == 6);
        
        // Verify residual of order 1 is 4 by matching it with a new sell order
        engine.submit_order(std::make_unique<Order>(3, "AAPL", Side::Sell, OrderType::Limit, 100, 4, 1002));
        REQUIRE(fills.size() == 2);
        CHECK(fills[1].fill_qty == 4);
        CHECK(fills[1].passive_id == 1);
    }

    SECTION("Scenario 4: Multi-level sweep") {
        engine.submit_order(std::make_unique<Order>(1, "AAPL", Side::Sell, OrderType::Limit, 101, 10, 1000));
        engine.submit_order(std::make_unique<Order>(2, "AAPL", Side::Sell, OrderType::Limit, 102, 10, 1001));
        engine.submit_order(std::make_unique<Order>(3, "AAPL", Side::Buy, OrderType::Market, 0, 15, 1002));
        
        REQUIRE(fills.size() == 2);
        CHECK(fills[0].fill_price == 101);
        CHECK(fills[0].fill_qty == 10);
        CHECK(fills[1].fill_price == 102);
        CHECK(fills[1].fill_qty == 5);
    }

    SECTION("Scenario 5: Cancel") {
        engine.submit_order(std::make_unique<Order>(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000));
        engine.cancel_order(1);
        engine.submit_order(std::make_unique<Order>(2, "AAPL", Side::Sell, OrderType::Limit, 100, 10, 1001));
        CHECK(fills.empty());
    }

    SECTION("Scenario 6: Cancel non-existent") {
        // Cancel an ID that was never submitted
        CHECK_NOTHROW(engine.cancel_order(999));
        
        // Cancel an order that was already fully filled
        engine.submit_order(std::make_unique<Order>(10, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000));
        engine.submit_order(std::make_unique<Order>(11, "AAPL", Side::Sell, OrderType::Limit, 100, 10, 1001));
        REQUIRE(fills.size() == 1);
        CHECK_NOTHROW(engine.cancel_order(10));
    }

    SECTION("Scenario 8: Partial Cancel") {
        engine.submit_order(std::make_unique<Order>(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000));
        engine.reduce_order(1, 4); // Remaining 6
        engine.submit_order(std::make_unique<Order>(2, "AAPL", Side::Sell, OrderType::Limit, 100, 10, 1001));
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].fill_qty == 6);
    }

    SECTION("Scenario 7: Replace and Time Priority") {
        // 1. Order at P
        engine.submit_order(std::make_unique<Order>(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000));
        // 2. Another order at P (should be behind 1)
        engine.submit_order(std::make_unique<Order>(2, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1001));
        
        // 3. Replace 1 with same price but new timestamp (should now be behind 2)
        engine.replace_order(1, 3, 100, 10, 1005);
        
        // 4. Submit sell for 10
        engine.submit_order(std::make_unique<Order>(4, "AAPL", Side::Sell, OrderType::Limit, 100, 10, 1006));
        
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].passive_id == 2); // Should match with 2, not 3 (the replacement for 1)
    }
}
