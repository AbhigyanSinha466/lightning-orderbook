#pragma once

#include <string_view>

namespace engine {
namespace utils {

void log_info(std::string_view msg);
void log_warn(std::string_view msg);
void log_error(std::string_view msg);

} // namespace utils
} // namespace engine
