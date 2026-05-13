#pragma once

#include "order.hpp"
#include <vector>
#include <cassert>
#include <memory>

namespace engine {
namespace utils {

class OrderPool {
public:
    explicit OrderPool(size_t initial_capacity = 65536) {
        expand(initial_capacity);
    }

    template <typename... Args>
    Order* allocate(Args&&... args) {
        if (free_list.empty()) {
            expand(capacity);
        }

        void* ptr = free_list.back();
        free_list.pop_back();
        return new (ptr) Order(std::forward<Args>(args)...);
    }

    void deallocate(Order* o) {
        if (!o) return;
        free_list.push_back(o);
    }

private:
    void expand(size_t count) {
        auto block = std::make_unique<uint8_t[]>(count * sizeof(Order));
        uint8_t* raw_ptr = block.get();
        for (size_t i = 0; i < count; ++i) {
            free_list.push_back(reinterpret_cast<Order*>(raw_ptr + (i * sizeof(Order))));
        }
        arenas.push_back(std::move(block));
        capacity += count;
    }

    size_t capacity = 0;
    std::vector<Order*> free_list;
    std::vector<std::unique_ptr<uint8_t[]>> arenas;
};

} // namespace utils
} // namespace engine
