#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "engine/engine.h"

using Catch::Matchers::WithinAbs;

namespace {

std::vector<chisel::Bar> trending_bars(std::size_t n, double start,
                                       double step) {
  std::vector<chisel::Bar> bars;
  bars.reserve(n);
  double price = start;
  for (std::size_t i = 0; i < n; ++i) {
    bars.push_back({price, price, 0});
    price += step;
  }
  return bars;
}

} // namespace

TEST_CASE("run_backtest returns a snapshot per bar except the last",
          "[chisel][engine]") {
  auto bars = trending_bars(10, 100.0, 0.5);
  auto result = chisel::run_backtest(bars, "TEST", 10'000.0);

  REQUIRE(result.snapshots.size() == 9);
  REQUIRE(result.summary.start_balance == 10'000.0);
}

TEST_CASE("run_backtest with no trades returns starting cash",
          "[chisel][engine]") {
  auto bars = trending_bars(30, 100.0, 0.0);
  auto result = chisel::run_backtest(bars, "TEST", 10'000.0);

  REQUIRE(result.summary.trade_count == 0);
  REQUIRE_THAT(result.summary.return_percent, WithinAbs(0.0, 1e-9));
}

TEST_CASE("run_backtest produces a valid summary on real-ish data",
          "[chisel][engine]") {
  std::vector<chisel::Bar> bars;
  bars.reserve(90);
  double price = 200.0;
  for (int i = 0; i < 60; ++i) {
    bars.push_back({price, price, 0});
    price -= 1.0;
  }
  for (int i = 0; i < 30; ++i) {
    bars.push_back({price, price, 0});
    price += 2.0;
  }
  auto result = chisel::run_backtest(bars, "TEST", 10'000.0);

  REQUIRE(result.summary.start_balance == 10'000.0);
  REQUIRE(result.summary.end_balance > 0.0);
  REQUIRE(result.snapshots.size() == bars.size() - 1);
}
