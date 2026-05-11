#pragma once

#include <queue>
#include <cstddef>

namespace engine {
namespace utils {

/**
 * @brief RingBuffer is a generic fixed-capacity FIFO queue.
 * Initial implementation wraps std::queue with a capacity limit.
 * Not thread-safe as per Section 11.4.
 */
template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity) : max_capacity(capacity) {}

    bool push(T value) {
        if (q.size() >= max_capacity) {
            return false;
        }
        q.push(std::move(value));
        return true;
    }

    bool pop(T& out) {
        if (q.empty()) {
            return false;
        }
        out = std::move(q.front());
        q.pop();
        return true;
    }

    size_t size() const {
        return q.size();
    }

    size_t capacity() const {
        return max_capacity;
    }

private:
    std::queue<T> q;
    size_t max_capacity;
};

} // namespace utils
} // namespace engine
