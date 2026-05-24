#pragma once

#include <string_view>
#include "types.hpp"

namespace engine {
namespace utils {

void log_info(std::string_view msg);
void log_warn(std::string_view msg);
void log_error(std::string_view msg);

void log_trade(const FillEvent& fill);

} // namespace utils
} // namespace engine