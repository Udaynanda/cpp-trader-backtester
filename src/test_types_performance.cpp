#include "types.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <cstring>

using namespace trading;

// Old-style tick with std::string
struct OldTick {
    std::string symbol;
    Price price;
    Quantity volume;
    Timestamp timestamp;
    Side side;
};

void benchmark_tick_copy() {
    std::cout << "=== Tick Copy Performance ===\n\n";
    
    constexpr size_t iterations = 10000000;
    
    // Benchmark new fixed-size tick
    {
        std::vector<Tick> ticks;
        ticks.reserve(iterations);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            ticks.emplace_back("AAPL", static_cast<Price>(1000000 + i), 100, 
                             static_cast<Timestamp>(i * 1000), Side::BUY);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Fixed-size Tick (char[16]):\n";
        std::cout << "  Iterations: " << iterations << "\n";
        std::cout << "  Total time: " << duration.count() << " ms\n";
        std::cout << "  Avg time: " << (duration.count() * 1000000.0 / iterations) << " ns/tick\n";
        std::cout << "  Memory: " << (ticks.size() * sizeof(Tick)) << " bytes\n\n";
    }
    
    // Benchmark old std::string tick
    {
        std::vector<OldTick> ticks;
        ticks.reserve(iterations);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            ticks.push_back(OldTick{"AAPL", static_cast<Price>(1000000 + i), 100, 
                                   static_cast<Timestamp>(i * 1000), Side::BUY});
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "std::string Tick:\n";
        std::cout << "  Iterations: " << iterations << "\n";
        std::cout << "  Total time: " << duration.count() << " ms\n";
        std::cout << "  Avg time: " << (duration.count() * 1000000.0 / iterations) << " ns/tick\n";
        std::cout << "  Memory (base): " << (ticks.size() * sizeof(OldTick)) << " bytes\n";
        std::cout << "  (+ heap allocations for strings)\n\n";
    }
}

void benchmark_order_fields() {
    std::cout << "=== Order Structure Analysis ===\n\n";
    
    Order order(1, 1000000, 100, 1000, Side::BUY, OrderType::LIMIT, 1);
    
    std::cout << "Order size: " << sizeof(Order) << " bytes\n";
    std::cout << "Order alignment: " << alignof(Order) << " bytes\n\n";
    
    // Test new fields
    order.filled = 30;
    std::cout << "Order tracking:\n";
    std::cout << "  Initial quantity: " << order.initial_quantity << "\n";
    std::cout << "  Current quantity: " << order.quantity << "\n";
    std::cout << "  Filled: " << order.filled << "\n";
    std::cout << "  Remaining: " << order.remaining() << "\n";
    std::cout << "  Fill ratio: " << (order.fill_ratio() * 100) << "%\n\n";
}

void benchmark_tick_size() {
    std::cout << "=== Memory Layout Analysis ===\n\n";
    
    std::cout << "Type sizes:\n";
    std::cout << "  Tick (new): " << sizeof(Tick) << " bytes\n";
    std::cout << "  OldTick: " << sizeof(OldTick) << " bytes\n";
    std::cout << "  Order: " << sizeof(Order) << " bytes\n";
    std::cout << "  Trade: " << sizeof(Trade) << " bytes\n\n";
    
    std::cout << "Cache line alignment:\n";
    std::cout << "  Tick: " << alignof(Tick) << " bytes\n";
    std::cout << "  Order: " << alignof(Order) << " bytes\n";
    std::cout << "  Trade: " << alignof(Trade) << " bytes\n\n";
    
    // Calculate ticks per cache line
    constexpr size_t cache_line = 64;
    std::cout << "Cache efficiency:\n";
    std::cout << "  Ticks per cache line: " << (cache_line / sizeof(Tick)) << "\n";
    std::cout << "  Orders per cache line: " << (cache_line / sizeof(Order)) << "\n\n";
}

void test_symbol_operations() {
    std::cout << "=== Symbol Operations ===\n\n";
    
    Tick tick1("AAPL", 1000000, 100, 1000, Side::BUY);
    Tick tick2("MSFT", 2000000, 200, 2000, Side::SELL);
    
    std::cout << "Tick 1 symbol: " << tick1.symbol << "\n";
    std::cout << "Tick 2 symbol: " << tick2.symbol << "\n\n";
    
    // Test long symbol
    Tick tick3("VERYLONGSYMBOLNAME", 3000000, 300, 3000, Side::BUY);
    std::cout << "Long symbol: " << tick3.symbol << "\n";
    std::cout << "Length: " << tick3.symbol.length() << " chars\n\n";
}

int main() {
    std::cout << "=== Types Performance & Correctness Tests ===\n\n";
    
    benchmark_tick_size();
    benchmark_order_fields();
    test_symbol_operations();
    benchmark_tick_copy();
    
    std::cout << "=== ALL TESTS COMPLETE ===\n";
    return 0;
}
