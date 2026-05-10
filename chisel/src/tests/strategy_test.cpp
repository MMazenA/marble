#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "strategy/ma_crossover.h"

TEST_CASE("MACrossover holds during warmup", "[chisel][strategy]") {
  chisel::MACrossover s(3, 5);
  for (int i = 0; i < 4; ++i) {
    REQUIRE(s.on_bar(100.0 + i) == chisel::Action::Hold);
  }
}

TEST_CASE("MACrossover detects golden cross (buy)", "[chisel][strategy]") {
  chisel::MACrossover s(3, 5);
  std::vector<double> closes{100, 99, 98, 97, 96, 95, 94, 93,
                              120, 130, 140};
  bool saw_buy = false;
  for (double c : closes) {
    if (s.on_bar(c) == chisel::Action::Buy) {
      saw_buy = true;
    }
  }
  REQUIRE(saw_buy);
}

TEST_CASE("MACrossover detects death cross (sell)", "[chisel][strategy]") {
  chisel::MACrossover s(3, 5);
  std::vector<double> closes{100, 105, 110, 115, 120, 125, 130, 135,
                              80, 70, 60};
  bool saw_sell = false;
  for (double c : closes) {
    if (s.on_bar(c) == chisel::Action::Sell) {
      saw_sell = true;
    }
  }
  REQUIRE(saw_sell);
}
