#pragma once

#include "types.hpp"
#include <vector>
#include <functional>
#include <cstdint>
#include <cstring>

namespace engine {
namespace itch {

// 9.3 Parsed Message Structs
struct AddOrderMsg {
    Timestamp timestamp;
    OrderId id;
    Side side;
    Qty quantity;
    Symbol symbol;
    Price price;
};

struct ReplaceOrderMsg {
    Timestamp timestamp;
    OrderId old_id;
    OrderId new_id;
    Qty new_quantity;
    Price new_price;
};

struct DeleteOrderMsg {
    Timestamp timestamp;
    OrderId id;
};

struct CancelOrderMsg {
    Timestamp timestamp;
    OrderId id;
    Qty cancelled_quantity;
};

struct ExecutedOrderMsg {
    Timestamp timestamp;
    OrderId id;
    Qty executed_quantity;
};

/**
 * @brief ItchParser parses NASDAQ ITCH 5.0 binary streams.
 * It uses a callback-based interface to dispatch parsed messages.
 */
class ItchParser {
public:
    using AddOrderCb = std::function<void(const AddOrderMsg&)>;
    using ReplaceOrderCb = std::function<void(const ReplaceOrderMsg&)>;
    using DeleteOrderCb = std::function<void(const DeleteOrderMsg&)>;
    using CancelOrderCb = std::function<void(const CancelOrderMsg&)>;
    using ExecutedOrderCb = std::function<void(const ExecutedOrderMsg&)>;

    void set_on_add_order(AddOrderCb cb) { on_add = std::move(cb); }
    void set_on_replace_order(ReplaceOrderCb cb) { on_replace = std::move(cb); }
    void set_on_delete_order(DeleteOrderCb cb) { on_delete = std::move(cb); }
    void set_on_cancel_order(CancelOrderCb cb) { on_cancel = std::move(cb); }
    void set_on_executed_order(ExecutedOrderCb cb) { on_executed = std::move(cb); }

    /**
     * @brief Parses a buffer of ITCH data.
     * @param data Pointer to the start of the data.
     * @param size Size of the data in bytes.
     * @return Number of bytes consumed.
     */
    size_t parse(const uint8_t* data, size_t size);

private:
    AddOrderCb on_add;
    ReplaceOrderCb on_replace;
    DeleteOrderCb on_delete;
    CancelOrderCb on_cancel;
    ExecutedOrderCb on_executed;
};

} // namespace itch
} // namespace engine
