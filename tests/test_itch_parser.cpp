#include <catch2/catch_test_macros.hpp>
#include "itch_parser.hpp"
#include <vector>

using namespace engine;
using namespace engine::itch;

TEST_CASE("ITCH Parser basic", "[itch]") {
    ItchParser parser;
    std::vector<AddOrderMsg> adds;
    std::vector<CancelOrderMsg> cancels;
    std::vector<ExecutedOrderMsg> executions;
    parser.set_on_add_order([&](const AddOrderMsg& msg) { adds.push_back(msg); });
    parser.set_on_cancel_order([&](const CancelOrderMsg& msg) { cancels.push_back(msg); });
    parser.set_on_executed_order([&](const ExecutedOrderMsg& msg) { executions.push_back(msg); });

    // Mock Add Order message ('A')
    // ... (rest of data remains same for 'A')
    // Length: 36 (0x0024)
    // Type: 'A' (0x41)
    // Locate: 0 (0x0000)
    // Tracking: 0 (0x0000)
    // Timestamp: 123456789 (0x0000075BCD15)
    // OrderId: 1 (0x0000000000000001)
    // Side: 'B' (0x42)
    // Shares: 100 (0x00000064)
    // Stock: "AAPL    " (41 41 50 4C 20 20 20 20)
    // Price: 150 (0x00000096)
    
    std::vector<uint8_t> data = {
        0x00, 0x24, // Length
        0x41, // Type
        0x00, 0x00, // Locate
        0x00, 0x00, // Tracking
        0x00, 0x00, 0x07, 0x5B, 0xCD, 0x15, // Timestamp (6 bytes)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // OrderId
        0x42, // Side 'B'
        0x00, 0x00, 0x00, 0x64, // Shares
        0x41, 0x41, 0x50, 0x4C, 0x20, 0x20, 0x20, 0x20, // Stock
        0x00, 0x00, 0x00, 0x96 // Price
    };

    SECTION("Full message") {
        size_t consumed = parser.parse(data.data(), data.size());
        CHECK(consumed == data.size());
        REQUIRE(adds.size() == 1);
        CHECK(adds[0].id == 1);
        CHECK(adds[0].side == Side::Buy);
        CHECK(adds[0].quantity == 100);
        CHECK(adds[0].symbol == "AAPL");
        CHECK(adds[0].price == 150);
        CHECK(adds[0].timestamp == 123456789);
    }

    SECTION("Partial message") {
        size_t consumed = parser.parse(data.data(), data.size() - 5);
        CHECK(consumed == 0);
        CHECK(adds.empty());
        
        // Feed ONLY the remaining 5 bytes. 
        consumed = parser.parse(data.data() + (data.size() - 5), 5);
        
        // Expected behavior for the current stateless parser:
        // It should NOT be able to parse the remaining 5 bytes as a new message
        // because it doesn't remember the first part.
        CHECK(consumed == 0);
        CHECK(adds.empty());
    }

    SECTION("Cancel Order (X)") {
        // Length: 23 (0x0017)
        // Type: 'X' (0x58)
        // Timestamp: 123456790 (0x0000075BCD16)
        // OrderId: 1 (0x0000000000000001)
        // Cancelled Shares: 50 (0x00000032)
        std::vector<uint8_t> cancel_data = {
            0x00, 0x17, // Length
            0x58, // Type 'X'
            0x00, 0x00, 0x00, 0x00, // Locate/Tracking
            0x00, 0x00, 0x07, 0x5B, 0xCD, 0x16, // Timestamp
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // OrderId
            0x00, 0x00, 0x00, 0x32 // Cancelled Shares
        };
        size_t consumed = parser.parse(cancel_data.data(), cancel_data.size());
        CHECK(consumed == cancel_data.size());
        REQUIRE(cancels.size() == 1);
        CHECK(cancels[0].id == 1);
        CHECK(cancels[0].cancelled_quantity == 50);
    }

    SECTION("Order Executed (E)") {
        // Length: 31 (0x001F)
        // Type: 'E' (0x45)
        // Timestamp: 123456791 (0x0000075BCD17)
        // OrderId: 1 (0x0000000000000001)
        // Executed Shares: 25 (0x00000019)
        // Match Number: 1000 (0x00000000000003E8)
        std::vector<uint8_t> exec_data = {
            0x00, 0x1F, // Length
            0x45, // Type 'E'
            0x00, 0x00, 0x00, 0x00, // Locate/Tracking
            0x00, 0x00, 0x07, 0x5B, 0xCD, 0x17, // Timestamp
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // OrderId
            0x00, 0x00, 0x00, 0x19, // Executed Shares
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xE8 // Match Number
        };
        size_t consumed = parser.parse(exec_data.data(), exec_data.size());
        CHECK(consumed == exec_data.size());
        REQUIRE(executions.size() == 1);
        CHECK(executions[0].id == 1);
        CHECK(executions[0].executed_quantity == 25);
    }
}
