#include <catch2/catch_test_macros.hpp>
#include "matching_engine.hpp"
#include <vector>

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

// Macro to abstract submission differences
#ifdef IS_F_VECTOR_IMPL
#define SUBMIT_ORDER(engine, id, sym, side, type, price, qty, ts) \
    engine.submit_order(Order(id, sym, side, type, price, qty, ts))
#else
#define SUBMIT_ORDER(engine, id, sym, side, type, price, qty, ts) \
    engine.submit_order(std::make_unique<Order>(id, sym, side, type, price, qty, ts))
#endif

TEST_CASE("MatchingEngine scenarios", "[matching_engine]") {
    MatchingEngine engine;
    std::vector<FillEvent> fills;
    engine.set_on_fill([&](const FillEvent& e) { fills.push_back(e); });

    SECTION("Scenario 1: No match") {
        SUBMIT_ORDER(engine, 1, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1000);
        SUBMIT_ORDER(engine, 2, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 101, 10, 1001);
        CHECK(fills.empty());
    }

    SECTION("Scenario 2: Exact match") {
        SUBMIT_ORDER(engine, 1, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1000);
        SUBMIT_ORDER(engine, 2, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 100, 10, 1001);
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].aggressive_id == 2);
        CHECK(fills[0].passive_id == 1);
        CHECK(fills[0].fill_qty == 10);
        CHECK(fills[0].fill_price == 100);
    }

    SECTION("Scenario 3: Partial fill") {
        SUBMIT_ORDER(engine, 1, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1000);
        SUBMIT_ORDER(engine, 2, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 100, 6, 1001);
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].fill_qty == 6);
        
        // Verify residual of order 1 is 4 by matching it with a new sell order
        SUBMIT_ORDER(engine, 3, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 100, 4, 1002);
        REQUIRE(fills.size() == 2);
        CHECK(fills[1].fill_qty == 4);
        CHECK(fills[1].passive_id == 1);
    }

    SECTION("Scenario 4: Multi-level sweep") {
        SUBMIT_ORDER(engine, 1, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 101, 10, 1000);
        SUBMIT_ORDER(engine, 2, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 102, 10, 1001);
        SUBMIT_ORDER(engine, 3, to_symbol("AAPL"), Side::Buy, OrderType::Market, 0, 15, 1002);
        
        REQUIRE(fills.size() == 2);
        CHECK(fills[0].fill_price == 101);
        CHECK(fills[0].fill_qty == 10);
        CHECK(fills[1].fill_price == 102);
        CHECK(fills[1].fill_qty == 5);
    }

    SECTION("Scenario 5: Cancel") {
        SUBMIT_ORDER(engine, 1, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1000);
        engine.cancel_order(1);
        SUBMIT_ORDER(engine, 2, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 100, 10, 1001);
        CHECK(fills.empty());
    }

    SECTION("Scenario 6: Cancel non-existent") {
        // Cancel an ID that was never submitted
        CHECK_NOTHROW(engine.cancel_order(999));
        
        // Cancel an order that was already fully filled
        SUBMIT_ORDER(engine, 10, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1000);
        SUBMIT_ORDER(engine, 11, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 100, 10, 1001);
        REQUIRE(fills.size() == 1);
        CHECK_NOTHROW(engine.cancel_order(10));
    }

    SECTION("Scenario 8: Partial Cancel") {
        SUBMIT_ORDER(engine, 1, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1000);
        engine.reduce_order(1, 4); // Remaining 6
        SUBMIT_ORDER(engine, 2, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 100, 10, 1001);
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].fill_qty == 6);
    }

    SECTION("Scenario 7: Replace and Time Priority") {
        // 1. Order at P
        SUBMIT_ORDER(engine, 1, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1000);
        // 2. Another order at P (should be behind 1)
        SUBMIT_ORDER(engine, 2, to_symbol("AAPL"), Side::Buy, OrderType::Limit, 100, 10, 1001);
        
        // 3. Replace 1 with same price but new timestamp (should now be behind 2)
        engine.replace_order(1, 3, 100, 10, 1005);
        
        // 4. Submit sell for 10
        SUBMIT_ORDER(engine, 4, to_symbol("AAPL"), Side::Sell, OrderType::Limit, 100, 10, 1006);
        
        REQUIRE(fills.size() == 1);
        CHECK(fills[0].passive_id == 2); // Should match with 2, not 3 (the replacement for 1)
    }
}
