#pragma once

#include "types.hpp"
#include "memory_pool.hpp"
#include <map>
#include <list>
#include <functional>
#include <vector>

namespace trading {

class OrderBook {
public:
    using TradeCallback = std::function<void(const Trade&)>;
    
    OrderBook(const std::string& symbol);
    
    // Core operations
    void add_order(Order* order);
    void cancel_order(OrderId order_id);
    void process_market_order(Order* order);
    
    // Getters
    Price best_bid() const { return bids_.empty() ? 0 : bids_.rbegin()->first; }
    Price best_ask() const { return asks_.empty() ? 0 : asks_.begin()->first; }
    Quantity bid_volume() const;
    Quantity ask_volume() const;
    
    void set_trade_callback(TradeCallback cb) { trade_callback_ = std::move(cb); }
    
    // Statistics
    size_t total_trades() const { return total_trades_; }
    
private:
    struct PriceLevel {
        Price price;
        std::list<Order*> orders;
        Quantity total_quantity = 0;
    };
    
    void match_order(Order* order);
    void execute_trade(Order* buy_order, Order* sell_order, Price price, Quantity qty);
    
    std::string symbol_;
    std::map<Price, PriceLevel, std::greater<Price>> bids_;  // Descending
    std::map<Price, PriceLevel> asks_;                        // Ascending
    TradeCallback trade_callback_;
    size_t total_trades_ = 0;
};

} // namespace trading
