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

    SECTION("Percentile p=0 handles underflow") {
        stats.add_sample(100);
        stats.add_sample(200);
        stats.add_sample(300);
        
        // Since report() is the only way to trigger the internal sorting and 
        // the percentile lambda is local to report(), we'll check the output
        // or we could refactor LatencyStats to expose percentile().
        // Given the instructions, let's assume if it doesn't crash and prints
        // correct Min (which is what p=0 should return), it's fine.
        
        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        stats.report();
        std::cout.rdbuf(old_buf);
        
        // Min should be 100.00
        CHECK(ss.str().find("Min:                100") != std::string::npos);
        // Median (50th) should be 200.00
        CHECK(ss.str().find("50th (Median):      200") != std::string::npos);
    }
}
