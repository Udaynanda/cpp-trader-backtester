#include "tick_engine.hpp"
#include "../strategies/momentum_strategy.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>

using namespace trading;

// Generate synthetic tick data
std::vector<Tick> generate_synthetic_ticks(size_t count) {
    std::vector<Tick> ticks;
    ticks.reserve(count);
    
    std::mt19937_64 rng(42);
    std::normal_distribution<> price_dist(0, 0.001);
    std::uniform_int_distribution<> vol_dist(100, 1000);
    std::bernoulli_distribution side_dist(0.5);
    
    Price base_price = 1000000; // $100.00
    Timestamp ts = 1700000000000000000ULL;
    
    for (size_t i = 0; i < count; ++i) {
        base_price += static_cast<Price>(price_dist(rng) * base_price);
        
        Tick tick{
            "AAPL",
            base_price,
            vol_dist(rng),
            ts,
            side_dist(rng) ? Side::BUY : Side::SELL
        };
        
        ticks.push_back(tick);
        ts += 1000000; // 1ms between ticks
    }
    
    return ticks;
}

// Load ticks from CSV
std::vector<Tick> load_ticks_from_csv(const std::string& filename) {
    std::vector<Tick> ticks;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Could not open " << filename << ", using synthetic data\n";
        return generate_synthetic_ticks(1000000);
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string symbol, side_str;
        double price;
        int64_t volume, timestamp;
        
        if (std::getline(ss, symbol, ',') &&
            ss >> timestamp && ss.ignore() &&
            ss >> price && ss.ignore() &&
            ss >> volume && ss.ignore() &&
            std::getline(ss, side_str)) {
            
            Tick tick{
                symbol,
                static_cast<Price>(price * 10000),
                volume,
                static_cast<Timestamp>(timestamp),
                side_str == "BUY" ? Side::BUY : Side::SELL
            };
            ticks.push_back(tick);
        }
    }
    
    return ticks;
}

int main(int argc, char** argv) {
    std::cout << "=== C++ Quantitative Trading Backtester ===\n\n";
    
    // Load or generate tick data
    std::vector<Tick> ticks;
    if (argc > 1) {
        ticks = load_ticks_from_csv(argv[1]);
    } else {
        std::cout << "Generating 1M synthetic ticks...\n";
        ticks = generate_synthetic_ticks(1000000);
    }
    
    std::cout << "Loaded " << ticks.size() << " ticks\n\n";
    
    // Create engine and strategies
    TickEngine engine;
    engine.add_strategy(std::make_unique<MomentumStrategy>(20));
    engine.add_strategy(std::make_unique<MarketMakerStrategy>(50));
    
    std::cout << "Running backtest...\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    engine.run_backtest(ticks);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Print statistics
    const auto& stats = engine.get_stats();
    std::cout << "\n=== Backtest Results ===\n";
    std::cout << "Ticks processed:    " << stats.ticks_processed << "\n";
    std::cout << "Orders submitted:   " << stats.orders_submitted << "\n";
    std::cout << "Trades executed:    " << stats.trades_executed << "\n";
    std::cout << "Total time:         " << duration.count() << " ms\n";
    std::cout << "Throughput:         " << (stats.ticks_processed * 1000.0 / duration.count()) 
              << " ticks/sec\n";
    std::cout << "Avg latency:        " << stats.avg_latency_us() << " µs/tick\n";
    
    return 0;
}
