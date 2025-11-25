#pragma once

#include "types.hpp"
#include "order_book.hpp"
#include "memory_pool.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace trading {

class Strategy;

class TickEngine {
public:
    TickEngine();
    
    // Event-driven simulation
    void process_tick(const Tick& tick);
    void submit_order(const Order& order);
    void run_backtest(const std::vector<Tick>& ticks);
    
    // Strategy management
    void add_strategy(std::unique_ptr<Strategy> strategy);
    
    // Statistics
    struct Stats {
        uint64_t ticks_processed = 0;
        uint64_t orders_submitted = 0;
        uint64_t trades_executed = 0;
        uint64_t total_latency_ns = 0;
        
        double avg_latency_us() const {
            return ticks_processed > 0 ? 
                   (total_latency_ns / static_cast<double>(ticks_processed)) / 1000.0 : 0.0;
        }
    };
    
    const Stats& get_stats() const { return stats_; }
    OrderBook* get_order_book(const std::string& symbol);
    
private:
    void on_trade(const Trade& trade);
    
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> order_books_;
    std::vector<std::unique_ptr<Strategy>> strategies_;
    MemoryPool<Order> order_pool_;
    OrderId next_order_id_ = 1;
    Timestamp current_time_ = 0;
    Stats stats_;
};

// Strategy interface
class Strategy {
public:
    virtual ~Strategy() = default;
    virtual void on_tick(const Tick& tick, TickEngine* engine) = 0;
    virtual void on_trade(const Trade& trade) = 0;
    virtual const char* name() const = 0;
};

} // namespace trading
