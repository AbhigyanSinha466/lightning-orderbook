#include "replay_harness.hpp"
#include "cycle_clock.hpp"

namespace engine {
namespace bench {

ReplayHarness::ReplayHarness(MatchingEngine& engine, itch::ItchParser& parser, LatencyStats& stats)
    : engine(engine), parser(parser), stats(stats) {
    parser.set_on_add_order([this](const itch::AddOrderMsg& msg) { on_add(msg); });
    parser.set_on_replace_order([this](const itch::ReplaceOrderMsg& msg) { on_replace(msg); });
    parser.set_on_delete_order([this](const itch::DeleteOrderMsg& msg) { on_delete(msg); });
    parser.set_on_cancel_order([this](const itch::CancelOrderMsg& msg) { on_cancel(msg); });
    parser.set_on_executed_order([this](const itch::ExecutedOrderMsg& msg) { on_executed(msg); });
}

void ReplayHarness::on_add(const itch::AddOrderMsg& msg) {
    auto start = CycleClock::get_cycles();
#ifdef IS_F_VECTOR_IMPL
    engine.submit_order(Order(msg.id, msg.symbol, msg.side, OrderType::Limit, msg.price, msg.quantity, msg.timestamp));
#else
    engine.submit_order(std::make_unique<Order>(msg.id, msg.symbol, msg.side, OrderType::Limit, msg.price, msg.quantity, msg.timestamp));
#endif
    auto end = CycleClock::get_cycles();
    stats.add_sample(end - start);
}

void ReplayHarness::on_replace(const itch::ReplaceOrderMsg& msg) {
    auto start = CycleClock::get_cycles();
    engine.replace_order(msg.old_id, msg.new_id, msg.new_price, msg.new_quantity, msg.timestamp);
    auto end = CycleClock::get_cycles();
    stats.add_sample(end - start);
}

void ReplayHarness::on_delete(const itch::DeleteOrderMsg& msg) {
    auto start = CycleClock::get_cycles();
    engine.cancel_order(msg.id);
    auto end = CycleClock::get_cycles();
    stats.add_sample(end - start);
}

void ReplayHarness::on_cancel(const itch::CancelOrderMsg& msg) {
    auto start = CycleClock::get_cycles();
    engine.reduce_order(msg.id, msg.cancelled_quantity);
    auto end = CycleClock::get_cycles();
    stats.add_sample(end - start);
}

void ReplayHarness::on_executed(const itch::ExecutedOrderMsg& msg) {
    auto start = CycleClock::get_cycles();
    engine.reduce_order(msg.id, msg.executed_quantity);
    auto end = CycleClock::get_cycles();
    stats.add_sample(end - start);
}

} // namespace bench
} // namespace engine
