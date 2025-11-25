#include "tick_engine.hpp"
#include <chrono>

namespace trading {

TickEngine::TickEngine() {}

void TickEngine::process_tick(const Tick& tick) {
    auto start = std::chrono::high_resolution_clock::now();
    
    current_time_ = tick.timestamp;
    
    // Get or create order book
    auto it = order_books_.find(tick.symbol);
    if (it == order_books_.end()) {
        auto ob = std::make_unique<OrderBook>(tick.symbol);
        ob->set_trade_callback([this](const Trade& t) { on_trade(t); });
        order_books_[tick.symbol] = std::move(ob);
        it = order_books_.find(tick.symbol);
    }
    
    // Notify strategies
    for (auto& strategy : strategies_) {
        strategy->on_tick(tick, this);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    ++stats_.ticks_processed;
    stats_.total_latency_ns += latency;
}

void TickEngine::submit_order(const Order& order_template) {
    Order* order = order_pool_.allocate();
    *order = order_template;
    order->id = next_order_id_++;
    order->timestamp = current_time_;
    
    // Use first available order book (in production, pass symbol with order)
    if (!order_books_.empty()) {
        order_books_.begin()->second->add_order(order);
        ++stats_.orders_submitted;
    }
}

void TickEngine::run_backtest(const std::vector<Tick>& ticks) {
    for (const auto& tick : ticks) {
        process_tick(tick);
    }
}

void TickEngine::add_strategy(std::unique_ptr<Strategy> strategy) {
    strategies_.push_back(std::move(strategy));
}

OrderBook* TickEngine::get_order_book(const std::string& symbol) {
    auto it = order_books_.find(symbol);
    return it != order_books_.end() ? it->second.get() : nullptr;
}

void TickEngine::on_trade(const Trade& trade) {
    ++stats_.trades_executed;
    for (auto& strategy : strategies_) {
        strategy->on_trade(trade);
    }
}

} // namespace trading
