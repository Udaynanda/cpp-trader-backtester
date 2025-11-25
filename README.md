# High-Performance C++ Trading Backtester

A low-latency, event-driven trading backtester with custom memory management and price-time priority order matching.

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Performance

- **34M ticks/sec** throughput
- **7.65M orders/sec** matching
- **1.69 ns** memory allocation
- **Sub-microsecond** latency

## Features

- Event-driven architecture with pluggable strategies
- Price-time priority order book (FIFO matching)
- Custom cache-aligned memory pool
- Market and limit orders with partial fills
- SIMD optimizations (ARM NEON / Intel AVX2)

## Quick Start

```bash
# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Run
./build/backtester              # Synthetic data
./build/backtester data.csv     # Your data
./build/benchmark               # Performance tests
```

## Usage

```cpp
#include "tick_engine.hpp"
#include "strategies/momentum_strategy.hpp"

int main() {
    TickEngine engine;
    engine.add_strategy(std::make_unique<MomentumStrategy>(20));
    
    auto ticks = load_ticks("data.csv");
    engine.run_backtest(ticks);
    
    const auto& stats = engine.get_stats();
    std::cout << "Trades: " << stats.trades_executed << "\n";
}
```

## Benchmark Results

Apple Silicon M1:

```
=== Memory Pool ===
Avg latency: 1.69 ns/allocation

=== Order Book ===
Throughput: 7.65M orders/sec
Avg latency: 0.131 µs/order
Trades: 77,334 from 100K orders

=== Tick Processing ===
Throughput: 34M ticks/sec
Avg latency: 0.015 µs/tick

=== Full Backtest (1M ticks, 2 strategies) ===
Orders submitted: 200,000
Trades executed: 101,799
Total time: 57 ms
Throughput: 17.5M ticks/sec
```

## Architecture

```
Tick Engine
    ├── Order Books (per symbol)
    │   └── Price-time priority matching
    ├── Strategies
    │   ├── Momentum (MA crossover)
    │   └── Market Maker (two-sided quotes)
    └── Memory Pool
        └── Cache-aligned allocator
```

## CSV Format

```csv
symbol,timestamp,price,volume,side
AAPL,1700000000000000000,150.25,100,BUY
AAPL,1700000000001000000,150.26,200,SELL
```

## Project Structure

```
├── include/          # Headers (types, order_book, tick_engine, memory_pool)
├── src/              # Implementation + tests
├── strategies/       # Trading strategies
└── data/            # Sample data
```

## Custom Strategy

```cpp
class MyStrategy : public Strategy {
    void on_tick(const Tick& tick, TickEngine* engine) override {
        if (should_buy(tick)) {
            Order order(0, tick.price, 100, tick.timestamp,
                       Side::BUY, OrderType::LIMIT, 1);
            engine->submit_order(order);
        }
    }
    
    void on_trade(const Trade& trade) override {
        // Track P&L
    }
    
    const char* name() const override { return "MyStrategy"; }
};
```

## Technical Highlights

- **Cache-aligned structures** (64-byte) for optimal CPU utilization
- **Fixed-point arithmetic** for deterministic behavior
- **Zero-copy design** for minimal overhead
- **Custom memory pool** eliminates malloc in hot path
- **SIMD instructions** for vectorized operations

## Testing

```bash
make test                    # All tests
./build/test_order_book     # Order book correctness
./build/test_strategies     # Strategy behavior
```

## Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) - Detailed system design and data flow

## License

MIT License - See [LICENSE](LICENSE)

## Author

Built to demonstrate low-latency systems programming and quantitative trading expertise.
