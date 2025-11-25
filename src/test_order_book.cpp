#include "order_book.hpp"
#include <iostream>
#include <cassert>

using namespace trading;

void test_partial_fill_volume() {
    std::cout << "Testing partial fill volume tracking...\n";
    
    OrderBook book("TEST");
    
    // Add sell order: 100 shares @ $100
    Order sell_order(1, 1000000, 100, 1000, Side::SELL, OrderType::LIMIT, 1);
    book.add_order(&sell_order);
    
    assert(book.ask_volume() == 100);
    assert(book.best_ask() == 1000000);
    std::cout << "  ✓ Initial ask volume: 100\n";
    
    // Buy 30 shares (partial fill)
    Order buy_order1(2, 1000000, 30, 2000, Side::BUY, OrderType::LIMIT, 2);
    book.add_order(&buy_order1);
    
    assert(book.ask_volume() == 70);  // Should be 70, not 100
    std::cout << "  ✓ After 30 share fill: " << book.ask_volume() << " (expected 70)\n";
    
    // Buy another 40 shares
    Order buy_order2(3, 1000000, 40, 3000, Side::BUY, OrderType::LIMIT, 3);
    book.add_order(&buy_order2);
    
    assert(book.ask_volume() == 30);  // Should be 30
    std::cout << "  ✓ After 40 share fill: " << book.ask_volume() << " (expected 30)\n";
    
    // Buy remaining 30 shares
    Order buy_order3(4, 1000000, 30, 4000, Side::BUY, OrderType::LIMIT, 4);
    book.add_order(&buy_order3);
    
    assert(book.ask_volume() == 0);  // Should be empty
    assert(book.best_ask() == 0);
    std::cout << "  ✓ After final fill: " << book.ask_volume() << " (expected 0)\n";
    
    std::cout << "✅ Partial fill volume tracking: PASSED\n\n";
}

void test_multiple_price_levels() {
    std::cout << "Testing multiple price level volume...\n";
    
    OrderBook book("TEST");
    
    // Add multiple sell orders at different prices
    Order sell1(1, 1000000, 100, 1000, Side::SELL, OrderType::LIMIT, 1);
    Order sell2(2, 1010000, 200, 1000, Side::SELL, OrderType::LIMIT, 1);
    Order sell3(3, 1020000, 300, 1000, Side::SELL, OrderType::LIMIT, 1);
    
    book.add_order(&sell1);
    book.add_order(&sell2);
    book.add_order(&sell3);
    
    assert(book.ask_volume() == 600);
    std::cout << "  ✓ Total ask volume: 600\n";
    
    // Market buy sweeps through levels
    Order market_buy(4, 0, 250, 2000, Side::BUY, OrderType::MARKET, 2);
    book.add_order(&market_buy);
    
    // Should consume all of level 1 (100) and half of level 2 (150)
    assert(book.ask_volume() == 350);  // 600 - 250 = 350
    assert(book.best_ask() == 1010000);  // Level 1 consumed
    std::cout << "  ✓ After market sweep: " << book.ask_volume() << " (expected 350)\n";
    
    std::cout << "✅ Multiple price level volume: PASSED\n\n";
}

void test_fifo_ordering() {
    std::cout << "Testing FIFO price-time priority...\n";
    
    OrderBook book("TEST");
    int trade_count = 0;
    
    book.set_trade_callback([&](const Trade& t) {
        trade_count++;
        std::cout << "  Trade " << trade_count << ": " 
                  << t.quantity << " @ " << (t.price / 10000.0) << "\n";
    });
    
    // Add 3 sell orders at same price (FIFO queue)
    Order sell1(1, 1000000, 100, 1000, Side::SELL, OrderType::LIMIT, 1);
    Order sell2(2, 1000000, 100, 2000, Side::SELL, OrderType::LIMIT, 2);
    Order sell3(3, 1000000, 100, 3000, Side::SELL, OrderType::LIMIT, 3);
    
    book.add_order(&sell1);
    book.add_order(&sell2);
    book.add_order(&sell3);
    
    // Market buy should match in FIFO order
    Order market_buy(4, 0, 250, 4000, Side::BUY, OrderType::MARKET, 4);
    book.add_order(&market_buy);
    
    assert(trade_count == 3);  // Should generate 3 trades
    assert(sell1.status == OrderStatus::FILLED);
    assert(sell2.status == OrderStatus::FILLED);
    assert(sell3.status == OrderStatus::PARTIAL);
    assert(sell3.filled == 50);
    
    std::cout << "  ✓ FIFO order preserved\n";
    std::cout << "✅ FIFO price-time priority: PASSED\n\n";
}

int main() {
    std::cout << "=== Order Book Correctness Tests ===\n\n";
    
    try {
        test_partial_fill_volume();
        test_multiple_price_levels();
        test_fifo_ordering();
        
        std::cout << "=== ALL TESTS PASSED ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ TEST FAILED: " << e.what() << "\n";
        return 1;
    }
}
