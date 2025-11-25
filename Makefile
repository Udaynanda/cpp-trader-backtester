.PHONY: all clean build run benchmark test

all: build

build:
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . -j

debug:
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . -j

run: build
	@./build/backtester

benchmark: build
	@./build/benchmark

test: build
	@./build/test_order_book

clean:
	@rm -rf build

help:
	@echo "Available targets:"
	@echo "  make build      - Build release version"
	@echo "  make debug      - Build debug version with sanitizers"
	@echo "  make run        - Build and run backtester"
	@echo "  make benchmark  - Build and run performance benchmarks"
	@echo "  make test       - Build and run correctness tests"
	@echo "  make clean      - Remove build artifacts"
