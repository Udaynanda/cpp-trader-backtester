#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <unordered_map>

namespace trading {

using Price = int64_t;      // Fixed-point: price * 10000
using Quantity = int64_t;
using OrderId = uint64_t;
using Timestamp = uint64_t; // Nanoseconds since epoch
using SymbolId = uint16_t;  // Symbol index for fast lookup

enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

enum class OrderType : uint8_t {
    MARKET = 0,
    LIMIT = 1
};

enum class OrderStatus : uint8_t {
    PENDING = 0,
    PARTIAL = 1,
    FILLED = 2,
    CANCELLED = 3
};

// Cache-aligned structures for performance
struct alignas(64) Order {
    OrderId id;
    Price price;
    Quantity quantity;
    Quantity filled;
    Quantity initial_quantity;  // Track original size for analytics
    Timestamp timestamp;
    Side side;
    OrderType type;
    OrderStatus status;
    uint32_t user_id;
    
    Order() = default;
    Order(OrderId id_, Price price_, Quantity qty_, Timestamp ts_, 
          Side side_, OrderType type_, uint32_t user_)
        : id(id_), price(price_), quantity(qty_), filled(0),
          initial_quantity(qty_), timestamp(ts_), side(side_), type(type_), 
          status(OrderStatus::PENDING), user_id(user_) {}
    
    // Helper methods
    Quantity remaining() const { return quantity - filled; }
    double fill_ratio() const { 
        return initial_quantity > 0 ? 
               static_cast<double>(filled) / initial_quantity : 0.0; 
    }
};

struct alignas(64) Trade {
    OrderId buy_order_id;
    OrderId sell_order_id;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
};

// Optimized tick structure - use std::string with SSO for best performance
struct Tick {
    std::string symbol;
    Price price;
    Quantity volume;
    Timestamp timestamp;
    Side side;
};

// Symbol registry for fast lookups (optional advanced feature)
class SymbolRegistry {
public:
    static SymbolRegistry& instance() {
        static SymbolRegistry reg;
        return reg;
    }
    
    SymbolId register_symbol(const std::string& symbol) {
        auto it = symbol_to_id_.find(symbol);
        if (it != symbol_to_id_.end()) {
            return it->second;
        }
        
        SymbolId id = static_cast<SymbolId>(symbols_.size());
        symbols_.push_back(symbol);
        symbol_to_id_[symbol] = id;
        return id;
    }
    
    const std::string& get_symbol(SymbolId id) const {
        return symbols_[id];
    }
    
private:
    std::vector<std::string> symbols_;
    std::unordered_map<std::string, SymbolId> symbol_to_id_;
};

} // namespace trading
