#include "tick_engine.hpp"
#include "../strategies/momentum_strategy.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace trading;

void test_momentum_strategy_signals() {
    std::cout << "Testing momentum strategy signal generation...\n";
    
    TickEngine engine;
    auto* strategy = new MomentumStrategy(5, 100);  // 5-tick window
    engine.add_strategy(std::unique_ptr<Strategy>(strategy));
    
    // Generate ticks with clear uptrend
    std::vector<Tick> ticks;
    Price base_price = 1000000;  // $100.00
    
    // First 5 ticks to build window (flat)
    for (int i = 0; i < 5; ++i) {
        ticks.push_back(Tick{"TEST", base_price, 100, static_cast<Timestamp>(i * 1000), Side::BUY});
    }
    
    // Next ticks show uptrend (should trigger buy)
    for (int i = 5; i < 10; ++i) {
        Price price = base_price + (i - 4) * 3000;  // +$0.30 per tick
        ticks.push_back(Tick{"TEST", price, 100, static_cast<Timestamp>(i * 1000), Side::BUY});
    }
    
    // Run backtest
    engine.run_backtest(ticks);
    
    const auto& stats = engine.get_stats();
    std::cout << "  Ticks processed: " << stats.ticks_processed << "\n";
    std::cout << "  Orders submitted: " << stats.orders_submitted << "\n";
    std::cout << "  Trades executed: " << stats.trades_executed << "\n";
    
    assert(stats.ticks_processed == 10);
    assert(stats.orders_submitted > 0);  // Should generate some orders
    
    std::cout << "✅ Momentum strategy signals: PASSED\n\n";
}

void test_market_maker_quoting() {
    std::cout << "Testing market maker quoting behavior...\n";
    
    TickEngine engine;
    auto* strategy = new MarketMakerStrategy(1000, 50, 500);  // $0.10 spread
    engine.add_strategy(std::unique_ptr<Strategy>(strategy));
    
    // Generate stable price ticks
    std::vector<Tick> ticks;
    Price mid_price = 1000000;  // $100.00
    
    for (int i = 0; i < 100; ++i) {
        ticks.push_back(Tick{"TEST", mid_price, 100, static_cast<Timestamp>(i * 1000), Side::BUY});
    }
    
    engine.run_backtest(ticks);
    
    const auto& stats = engine.get_stats();
    std::cout << "  Ticks processed: " << stats.ticks_processed << "\n";
    std::cout << "  Orders submitted: " << stats.orders_submitted << "\n";
    
    // Market maker quotes every 10 ticks, both sides
    // 100 ticks / 10 = 10 quote cycles * 2 sides = 20 orders
    assert(stats.ticks_processed == 100);
    assert(stats.orders_submitted == 20);  // 10 cycles * 2 sides
    
    std::cout << "✅ Market maker quoting: PASSED\n\n";
}

void test_strategy_position_tracking() {
    std::cout << "Testing strategy position tracking...\n";
    
    TickEngine engine;
    
    // Create order book for matching
    auto* book = engine.get_order_book("TEST");
    if (!book) {
        // Create by processing a tick
        Tick init_tick{"TEST", 1000000, 100, 0, Side::BUY};
        engine.process_tick(init_tick);
        book = engine.get_order_book("TEST");
    }
    
    int trade_count = 0;
    book->set_trade_callback([&](const Trade& t) {
        trade_count++;
        std::cout << "  Trade: " << t.quantity << " @ " 
                  << (t.price / 10000.0) << "\n";
    });
    
    // Add liquidity to book
    Order sell1(1, 1000000, 100, 1000, Side::SELL, OrderType::LIMIT, 99);
    Order sell2(2, 1010000, 100, 1000, Side::SELL, OrderType::LIMIT, 99);
    book->add_order(&sell1);
    book->add_order(&sell2);
    
    // Strategy submits buy order
    Order buy(3, 1000000, 50, 2000, Side::BUY, OrderType::LIMIT, 1);
    book->add_order(&buy);
    
    assert(trade_count == 1);
    assert(buy.filled == 50);
    assert(buy.status == OrderStatus::FILLED);
    
    std::cout << "✅ Position tracking: PASSED\n\n";
}

void test_multiple_strategies() {
    std::cout << "Testing multiple concurrent strategies...\n";
    
    TickEngine engine;
    engine.add_strategy(std::make_unique<MomentumStrategy>(10, 100));
    engine.add_strategy(std::make_unique<MarketMakerStrategy>(500, 25, 300));
    
    // Generate mixed market conditions
    std::vector<Tick> ticks;
    Price price = 1000000;
    
    for (int i = 0; i < 200; ++i) {
        // Add some volatility
        price += (i % 3 == 0) ? 1000 : -500;
        ticks.push_back(Tick{"TEST", price, 100, static_cast<Timestamp>(i * 1000), Side::BUY});
    }
    
    engine.run_backtest(ticks);
    
    const auto& stats = engine.get_stats();
    std::cout << "  Ticks processed: " << stats.ticks_processed << "\n";
    std::cout << "  Orders submitted: " << stats.orders_submitted << "\n";
    std::cout << "  Trades executed: " << stats.trades_executed << "\n";
    
    assert(stats.ticks_processed == 200);
    assert(stats.orders_submitted > 0);  // Both strategies should trade
    
    std::cout << "✅ Multiple strategies: PASSED\n\n";
}

int main() {
    std::cout << "=== Strategy Correctness Tests ===\n\n";
    
    try {
        test_momentum_strategy_signals();
        test_market_maker_quoting();
        test_strategy_position_tracking();
        test_multiple_strategies();
        
        std::cout << "=== ALL STRATEGY TESTS PASSED ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ TEST FAILED: " << e.what() << "\n";
        return 1;
    }
}
