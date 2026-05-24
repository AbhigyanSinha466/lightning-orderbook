#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"

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

TEST_CASE("OrderBook operations", "[order_book]") {
    OrderBook book;

    Symbol symbol = to_symbol("AAPL");
    Order b1(1, symbol, Side::Buy, OrderType::Limit, 100, 10, 1000);
    Order b2(2, symbol, Side::Buy, OrderType::Limit, 99, 20, 1001);
    Order a1(3, symbol, Side::Sell, OrderType::Limit, 101, 10, 1002);
    Order a2(4, symbol, Side::Sell, OrderType::Limit, 102, 20, 1003);

    SECTION("Best prices and spread") {
        book.add_order(&b1);
        book.add_order(&b2);
        book.add_order(&a1);
        book.add_order(&a2);

        CHECK(book.best_bid_price() == 100);
        CHECK(book.best_ask_price() == 101);
        CHECK(book.spread() == 1);
    }

    SECTION("Removing price levels") {
        book.add_order(&b1);
        book.add_order(&b2);
        CHECK(book.best_bid_price() == 100);
        book.remove_order(&b1);
        CHECK(book.best_bid_price() == 99);
        book.remove_order(&b2);
        CHECK(book.best_bid_price() == std::numeric_limits<Price>::min());
    }
}
