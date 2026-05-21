#pragma once

#include <atomic>
#include <cstddef>
#include <utility>

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

namespace engine {
namespace utils {

template <typename T, size_t BatchSize = 64>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity) : max_capacity(capacity), buffer(new T[capacity]()) {
        mask = capacity - 1;
    }
    ~RingBuffer() { delete[] buffer; }

    bool push(const T& value) {
        auto current_write = mLocalWriteCounter;
        if (current_write - mCachedReadCounter >= max_capacity) {
            mCachedReadCounter = mReadCounter.load(std::memory_order_acquire);
            if (current_write - mCachedReadCounter >= max_capacity) {
                return false;
            }
        }
        buffer[current_write & mask] = value;
        mLocalWriteCounter = current_write + 1;
        
        if (mLocalWriteCounter - mPublishedWriteCounter >= BatchSize) {
            mWriteCounter.store(mLocalWriteCounter, std::memory_order_release);
            mPublishedWriteCounter = mLocalWriteCounter;
        }
        return true;
    }

    void flush() {
        if (mLocalWriteCounter != mPublishedWriteCounter) {
            mWriteCounter.store(mLocalWriteCounter, std::memory_order_release);
            mPublishedWriteCounter = mLocalWriteCounter;
        }
    }

    bool pop(T& out) {
        auto current_read = mLocalReadCounter;
        if (current_read == mCachedWriteCounter) {
            mCachedWriteCounter = mWriteCounter.load(std::memory_order_acquire);
            if (current_read == mCachedWriteCounter) {
                return false;
            }
        }
        out = std::move(buffer[current_read & mask]);
        mLocalReadCounter = current_read + 1;
        
        if (mLocalReadCounter - mPublishedReadCounter >= BatchSize) {
            mReadCounter.store(mLocalReadCounter, std::memory_order_release);
            mPublishedReadCounter = mLocalReadCounter;
        }
        return true;
    }

    void consumer_flush() {
        if (mLocalReadCounter != mPublishedReadCounter) {
            mReadCounter.store(mLocalReadCounter, std::memory_order_release);
            mPublishedReadCounter = mLocalReadCounter;
        }
    }

    size_t size() const {
        return mWriteCounter.load(std::memory_order_relaxed) - mReadCounter.load(std::memory_order_relaxed);
    }

    size_t capacity() const {
        return max_capacity;
    }

private:
    // Producer cacheline
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> mWriteCounter{0};
    size_t mLocalWriteCounter{0};
    size_t mPublishedWriteCounter{0};
    size_t mCachedReadCounter{0};
    size_t max_capacity;
    size_t mask;
    T* buffer;

    // Consumer cacheline
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> mReadCounter{0};
    size_t mLocalReadCounter{0};
    size_t mPublishedReadCounter{0};
    size_t mCachedWriteCounter{0};
};

} // namespace utils
} // namespace engine
