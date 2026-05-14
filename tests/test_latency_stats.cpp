#include <catch2/catch_test_macros.hpp>
#include "latency_stats.hpp"
#include <sstream>
#include <iostream>

using namespace engine::bench;

TEST_CASE("LatencyStats percentile calculation", "[bench]") {
    LatencyStats stats;
    
    SECTION("Empty stats") {
        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        stats.report();
        std::cout.rdbuf(old_buf);
        CHECK(ss.str().find("No samples collected.") != std::string::npos);
    }

    SECTION("Percentile calculation") {
        stats.add_sample(100);
        stats.add_sample(200);
        stats.add_sample(300);
        
        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        stats.report();
        std::cout.rdbuf(old_buf);
        
        // Min should be 100
        CHECK(ss.str().find("Min:                100") != std::string::npos);
        // Median (50th) should be 200
        CHECK(ss.str().find("50th (Median):      200") != std::string::npos);
    }
}
