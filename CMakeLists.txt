cmake_minimum_required(VERSION 3.15)
project(TradingBacktester CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG -flto")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address")

# Enable SIMD
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
    # ARM NEON optimizations for Apple Silicon
    add_compile_options(-mcpu=apple-m1)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Intel AVX2 optimizations
    add_compile_options(-mavx2 -mfma)
endif()

include_directories(include)

# Core library
add_library(backtester_core
    src/order_book.cpp
    src/tick_engine.cpp
    src/memory_pool.cpp
)

# Main executable
add_executable(backtester
    src/main.cpp
)

target_link_libraries(backtester backtester_core pthread)

# Benchmark executable
add_executable(benchmark
    src/benchmark.cpp
)

target_link_libraries(benchmark backtester_core pthread)

# Test executables
add_executable(test_order_book
    src/test_order_book.cpp
)

target_link_libraries(test_order_book backtester_core pthread)

add_executable(test_strategies
    src/test_strategies.cpp
)

target_link_libraries(test_strategies backtester_core pthread)

add_executable(test_types
    src/test_types_performance.cpp
)

target_link_libraries(test_types backtester_core pthread)
