#include "itch_parser.hpp"
#include <algorithm>

namespace engine {
namespace itch {

namespace {
template <typename T>
T read_be(const uint8_t* data, size_t size = sizeof(T)) {
    T val = 0;
    for (size_t i = 0; i < size; ++i) {
        val = (val << 8) | data[i];
    }
    return val;
}

Symbol parse_symbol(const uint8_t* data) {
    uint64_t hash = 0;
    for (int i = 0; i < 8; ++i) {
        hash = hash * 31 + data[i];
    }
    return hash;
}
} // namespace

size_t ItchParser::parse(const uint8_t* data, size_t size) {
    size_t consumed = 0;
    while (consumed + 2 <= size) {
        uint16_t msg_len = read_be<uint16_t>(data + consumed);
        if (consumed + 2 + msg_len > size) {
            break; // Incomplete message
        }

        const uint8_t* body = data + consumed + 2;
        char type = static_cast<char>(body[0]);
        Timestamp ts = read_be<uint64_t>(body + 5, 6); // Offset 5, Length 6

        switch (type) {
            case 'A': { // Add Order (No MPID)
                if (on_add) {
                    AddOrderMsg msg;
                    msg.timestamp = ts;
                    msg.id = read_be<uint64_t>(body + 11);
                    msg.side = (body[19] == 'B' ? Side::Buy : Side::Sell);
                    msg.quantity = read_be<uint32_t>(body + 20);
                    msg.symbol = parse_symbol(body + 24);
                    msg.price = read_be<uint32_t>(body + 32);
                    on_add(msg);
                }
                break;
            }
            case 'U': { // Order Replace
                if (on_replace) {
                    ReplaceOrderMsg msg;
                    msg.timestamp = ts;
                    msg.old_id = read_be<uint64_t>(body + 11);
                    msg.new_id = read_be<uint64_t>(body + 19);
                    msg.new_quantity = read_be<uint32_t>(body + 27);
                    msg.new_price = read_be<uint32_t>(body + 31);
                    on_replace(msg);
                }
                break;
            }
            case 'D': { // Order Delete
                if (on_delete) {
                    DeleteOrderMsg msg;
                    msg.timestamp = ts;
                    msg.id = read_be<uint64_t>(body + 11);
                    on_delete(msg);
                }
                break;
            }
            case 'X': { // Order Cancel
                if (on_cancel) {
                    CancelOrderMsg msg;
                    msg.timestamp = ts;
                    msg.id = read_be<uint64_t>(body + 11);
                    msg.cancelled_quantity = read_be<uint32_t>(body + 19);
                    on_cancel(msg);
                }
                break;
            }
            case 'E': { // Order Executed
                if (on_executed) {
                    ExecutedOrderMsg msg;
                    msg.timestamp = ts;
                    msg.id = read_be<uint64_t>(body + 11);
                    msg.executed_quantity = read_be<uint32_t>(body + 19);
                    on_executed(msg);
                }
                break;
            }
            default:
                // Skip unknown message type
                break;
        }

        consumed += 2 + msg_len;
    }
    return consumed;
}

} // namespace itch
} // namespace engine
