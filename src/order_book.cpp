#include "order_book.hpp"
#include <algorithm>

namespace trading {

OrderBook::OrderBook(const std::string& symbol) : symbol_(symbol) {}

void OrderBook::add_order(Order* order) {
    if (order->type == OrderType::MARKET) {
        process_market_order(order);
        return;
    }
    
    match_order(order);
    
    // Add remaining quantity to book
    if (order->status != OrderStatus::FILLED) {
        if (order->side == Side::BUY) {
            auto& level = bids_[order->price];
            level.price = order->price;
            level.orders.push_back(order);
            level.total_quantity += (order->quantity - order->filled);
        } else {
            auto& level = asks_[order->price];
            level.price = order->price;
            level.orders.push_back(order);
            level.total_quantity += (order->quantity - order->filled);
        }
    }
}

void OrderBook::cancel_order(OrderId order_id) {
    // Simplified: In production, maintain order_id -> order* map
}

void OrderBook::process_market_order(Order* order) {
    match_order(order);
    if (order->status != OrderStatus::FILLED) {
        order->status = OrderStatus::CANCELLED; // No liquidity
    }
}

void OrderBook::match_order(Order* order) {
    if (order->side == Side::BUY) {
        // Match against asks
        while (order->filled < order->quantity && !asks_.empty()) {
            auto it = asks_.begin();
            auto& level = it->second;
        
            // Check price compatibility
            if (order->type == OrderType::LIMIT) {
                if (order->price < level.price) break;
            }
            
            while (!level.orders.empty() && order->filled < order->quantity) {
                Order* contra_order = level.orders.front();
                Quantity trade_qty = std::min(
                    order->quantity - order->filled,
                    contra_order->quantity - contra_order->filled
                );
                
                execute_trade(order, contra_order, level.price, trade_qty);
                
                order->filled += trade_qty;
                contra_order->filled += trade_qty;
                level.total_quantity -= trade_qty;
                
                if (contra_order->filled >= contra_order->quantity) {
                    contra_order->status = OrderStatus::FILLED;
                    level.orders.pop_front();
                } else {
                    contra_order->status = OrderStatus::PARTIAL;
                }
            }
            
            if (level.orders.empty()) {
                asks_.erase(it);
            }
        }
    } else {
        // Match against bids
        while (order->filled < order->quantity && !bids_.empty()) {
            auto it = bids_.begin();
            auto& level = it->second;
            
            // Check price compatibility
            if (order->type == OrderType::LIMIT) {
                if (order->price > level.price) break;
            }
            
            while (!level.orders.empty() && order->filled < order->quantity) {
                Order* contra_order = level.orders.front();
                Quantity trade_qty = std::min(
                    order->quantity - order->filled,
                    contra_order->quantity - contra_order->filled
                );
                
                execute_trade(contra_order, order, level.price, trade_qty);
                
                order->filled += trade_qty;
                contra_order->filled += trade_qty;
                level.total_quantity -= trade_qty;
                
                if (contra_order->filled >= contra_order->quantity) {
                    contra_order->status = OrderStatus::FILLED;
                    level.orders.pop_front();
                } else {
                    contra_order->status = OrderStatus::PARTIAL;
                }
            }
            
            if (level.orders.empty()) {
                bids_.erase(it);
            }
        }
    }
    
    order->status = (order->filled >= order->quantity) ? 
                    OrderStatus::FILLED : 
                    (order->filled > 0 ? OrderStatus::PARTIAL : OrderStatus::PENDING);
}

void OrderBook::execute_trade(Order* buy_order, Order* sell_order, 
                              Price price, Quantity qty) {
    Trade trade{
        buy_order->id,
        sell_order->id,
        price,
        qty,
        std::max(buy_order->timestamp, sell_order->timestamp)
    };
    
    if (trade_callback_) {
        trade_callback_(trade);
    }
    
    ++total_trades_;
}

Quantity OrderBook::bid_volume() const {
    Quantity vol = 0;
    for (const auto& [price, level] : bids_) {
        vol += level.total_quantity;
    }
    return vol;
}

Quantity OrderBook::ask_volume() const {
    Quantity vol = 0;
    for (const auto& [price, level] : asks_) {
        vol += level.total_quantity;
    }
    return vol;
}

} // namespace trading
