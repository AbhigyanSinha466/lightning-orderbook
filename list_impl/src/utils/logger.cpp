#include "utils/logger.hpp"
#include "utils/ring_buffer.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <atomic>

namespace engine {
namespace utils {

namespace {

void log(std::string_view level, std::string_view msg) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::cerr << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") 
              << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
              << "[" << level << "] " << msg << std::endl;
}

class AsyncLogger {
public:
    AsyncLogger() : trades_queue_(8 * 1024 * 1024), running_(true), logger_thread_(&AsyncLogger::run, this) {}

    ~AsyncLogger() {
        running_.store(false, std::memory_order_relaxed);
        trades_queue_.flush();
        if (logger_thread_.joinable()) {
            logger_thread_.join();
        }
    }

    void log_trade(const FillEvent& fill) {
        while (!trades_queue_.push(fill)) {
            // Spin until space is available
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

private:
    void run() {
        std::ofstream outfile("trades.log", std::ios::app);
        FillEvent fill;
        while (running_.load(std::memory_order_relaxed)) {
            bool popped = false;
            while (trades_queue_.pop(fill)) {
                popped = true;
                outfile << "Trade: Passive=" << fill.passive_id 
                        << " Aggressive=" << fill.aggressive_id 
                        << " Price=" << fill.fill_price 
                        << " Qty=" << fill.fill_qty << "\n";
            }
            if (!popped) {
                // Sleep when queue is empty to avoid hammering the cache line of mWriteCounter
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
        // Drain the queue before exiting
        while (trades_queue_.pop(fill)) {
            outfile << "Trade: Passive=" << fill.passive_id 
                    << " Aggressive=" << fill.aggressive_id 
                    << " Price=" << fill.fill_price 
                    << " Qty=" << fill.fill_qty << "\n";
        }
    }

    RingBuffer<FillEvent> trades_queue_;
    std::atomic<bool> running_;
    std::thread logger_thread_;
};

AsyncLogger& get_logger() {
    static AsyncLogger instance;
    return instance;
}

// Eager initialization to avoid thread startup latency during benchmark
struct LoggerInitializer {
    LoggerInitializer() {
        get_logger(); 
    }
} initializer;

} // namespace

void log_info(std::string_view msg) { log("INFO", msg); }
void log_warn(std::string_view msg) { log("WARN", msg); }
void log_error(std::string_view msg) { log("ERROR", msg); }

void log_trade(const FillEvent& fill) {
    get_logger().log_trade(fill);
}

} // namespace utils
} // namespace engine
