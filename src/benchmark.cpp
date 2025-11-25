#include "tick_engine.hpp"
#include "order_book.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>

using namespace trading;

void benchmark_order_book() {
    std::cout << "=== Order Book Benchmark ===\n";
    
    OrderBook book("TEST");
    std::vector<Order> orders;
    orders.reserve(100000);
    
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<Price> price_dist(990000, 1010000);
    std::uniform_int_distribution<Quantity> qty_dist(1, 100);
    std::bernoulli_distribution side_dist(0.5);
    
    // Pre-generate orders
    for (size_t i = 0; i < 100000; ++i) {
        orders.emplace_back(
            i,
            price_dist(rng),
            qty_dist(rng),
            i * 1000,
            side_dist(rng) ? Side::BUY : Side::SELL,
            OrderType::LIMIT,
            1
        );
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (auto& order : orders) {
        book.add_order(&order);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Orders processed: 100,000\n";
    std::cout << "Total time: " << duration.count() << " µs\n";
    std::cout << "Avg latency: " << (duration.count() / 100000.0) << " µs/order\n";
    std::cout << "Throughput: " << (100000.0 * 1000000.0 / duration.count()) << " orders/sec\n";
    std::cout << "Trades executed: " << book.total_trades() << "\n\n";
}

void benchmark_memory_pool() {
    std::cout << "=== Memory Pool Benchmark ===\n";
    
    MemoryPool<Order> pool;
    constexpr size_t iterations = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < iterations; ++i) {
        Order* order = pool.allocate();
        order->id = i;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "Allocations: " << iterations << "\n";
    std::cout << "Total time: " << (duration.count() / 1000000.0) << " ms\n";
    std::cout << "Avg latency: " << (duration.count() / static_cast<double>(iterations)) << " ns/allocation\n\n";
}

void benchmark_tick_processing() {
    std::cout << "=== Tick Processing Benchmark ===\n";
    
    TickEngine engine;
    std::vector<Tick> ticks;
    constexpr size_t tick_count = 10000000;
    ticks.reserve(tick_count);
    
    std::mt19937_64 rng(42);
    std::normal_distribution<> price_dist(0, 0.0001);
    
    Price price = 1000000;
    for (size_t i = 0; i < tick_count; ++i) {
        price += static_cast<Price>(price_dist(rng) * price);
        ticks.push_back(Tick{"AAPL", price, 100, i * 1000, Side::BUY});
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    engine.run_backtest(ticks);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Ticks processed: " << tick_count << "\n";
    std::cout << "Total time: " << duration.count() << " ms\n";
    std::cout << "Throughput: " << (tick_count * 1000.0 / duration.count()) << " ticks/sec\n";
    std::cout << "Avg latency: " << engine.get_stats().avg_latency_us() << " µs/tick\n\n";
}

int main() {
    std::cout << "=== Trading Engine Performance Benchmarks ===\n\n";
    
    benchmark_memory_pool();
    benchmark_order_book();
    benchmark_tick_processing();
    
    return 0;
}
