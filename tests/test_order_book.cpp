#include <catch2/catch_test_macros.hpp>
#include "order_book.hpp"

using namespace engine;

TEST_CASE("OrderBook operations", "[order_book]") {
    OrderBook book;

    Order b1(1, "AAPL", Side::Buy, OrderType::Limit, 100, 10, 1000);
    Order b2(2, "AAPL", Side::Buy, OrderType::Limit, 99, 20, 1001);
    Order a1(3, "AAPL", Side::Sell, OrderType::Limit, 101, 10, 1002);
    Order a2(4, "AAPL", Side::Sell, OrderType::Limit, 102, 20, 1003);

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
