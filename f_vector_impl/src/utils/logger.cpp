#include "utils/logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

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
} // namespace

void log_info(std::string_view msg) { log("INFO", msg); }
void log_warn(std::string_view msg) { log("WARN", msg); }
void log_error(std::string_view msg) { log("ERROR", msg); }

} // namespace utils
} // namespace engine
