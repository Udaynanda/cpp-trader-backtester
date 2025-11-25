#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace trading {

// Custom memory pool for order allocation with cache-line alignment
// BlockSize = number of objects per block (not bytes)
template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    static constexpr size_t CACHE_LINE_SIZE = 64;
    
    MemoryPool() : current_block_(0), current_index_(0) {
        allocate_block();
    }

    ~MemoryPool() {
        for (auto& block : blocks_) {
            if (block.ptr) {
                std::free(block.ptr);
            }
        }
    }

    // Fast allocation - no construction for POD types
    T* allocate() {
        if (current_index_ >= BlockSize) {
            allocate_block();
        }
        return &blocks_[current_block_].ptr[current_index_++];
    }

    // Reset pool for reuse (doesn't free memory)
    void reset() {
        current_block_ = 0;
        current_index_ = 0;
    }
    
    // Get total allocated memory in bytes
    size_t memory_usage() const {
        return blocks_.size() * BlockSize * sizeof(T);
    }
    
    // Get number of allocated objects
    size_t allocated_count() const {
        return current_block_ * BlockSize + current_index_;
    }

private:
    struct alignas(CACHE_LINE_SIZE) Block {
        T* ptr = nullptr;
    };
    
    void allocate_block() {
        if (current_block_ >= blocks_.size()) {
            Block block;
            
            // Allocate cache-line aligned memory
            size_t alloc_size = sizeof(T) * BlockSize;
            size_t aligned_size = (alloc_size + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
            void* raw_ptr = std::aligned_alloc(CACHE_LINE_SIZE, aligned_size);
            
            if (!raw_ptr) {
                throw std::bad_alloc();
            }
            
            block.ptr = static_cast<T*>(raw_ptr);
            blocks_.push_back(block);
        }
        current_block_ = blocks_.size() - 1;
        current_index_ = 0;
    }

    std::vector<Block> blocks_;
    size_t current_block_;
    size_t current_index_;
};

} // namespace trading
