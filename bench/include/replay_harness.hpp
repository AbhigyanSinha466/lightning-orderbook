#pragma once

#include "itch_parser.hpp"
#include "matching_engine.hpp"
#include "latency_stats.hpp"
#include <chrono>

namespace engine {
namespace bench {

/**
 * @brief ReplayHarness coordinates between the ITCH parser and the matching engine.
 * It measures the latency of processing each ITCH message.
 */
class ReplayHarness {
public:
    ReplayHarness(MatchingEngine& engine, itch::ItchParser& parser, LatencyStats& stats);

    /**
     * @brief Processes a message and records latency.
     * Logic is primarily in the callbacks registered during construction.
     */
private:
    MatchingEngine& engine;
    itch::ItchParser& parser;
    LatencyStats& stats;

    using Clock = std::chrono::high_resolution_clock;

    void on_add(const itch::AddOrderMsg& msg);
    void on_replace(const itch::ReplaceOrderMsg& msg);
    void on_delete(const itch::DeleteOrderMsg& msg);
    void on_cancel(const itch::CancelOrderMsg& msg);
    void on_executed(const itch::ExecutedOrderMsg& msg);
};

} // namespace bench
} // namespace engine
