#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "engine/account.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("Account starts with given cash", "[chisel][account]") {
  chisel::Account a(50'000.0);
  REQUIRE(a.cash == 50'000.0);
  REQUIRE(a.shares == 0);
  REQUIRE(a.realized_pnl == 0.0);
  REQUIRE(a.trade_count == 0);
}

TEST_CASE("buy decrements cash and adds shares", "[chisel][account]") {
  chisel::Account a(10'000.0);
  a.buy(10, 100.0);
  REQUIRE_THAT(a.cash, WithinAbs(9'000.0, 1e-9));
  REQUIRE(a.shares == 10);
  REQUIRE(a.avg_buy_price == 100.0);
  REQUIRE(a.trade_count == 0);
}

TEST_CASE("sell realizes PnL and counts trade", "[chisel][account]") {
  chisel::Account a(10'000.0);
  a.buy(10, 100.0);
  a.sell(10, 110.0);
  REQUIRE_THAT(a.cash, WithinAbs(10'100.0, 1e-9));
  REQUIRE_THAT(a.realized_pnl, WithinAbs(100.0, 1e-9));
  REQUIRE(a.shares == 0);
  REQUIRE(a.trade_count == 1);
}

TEST_CASE("losing sell records negative PnL", "[chisel][account]") {
  chisel::Account a(10'000.0);
  a.buy(5, 200.0);
  a.sell(5, 180.0);
  REQUIRE_THAT(a.realized_pnl, WithinAbs(-100.0, 1e-9));
  REQUIRE(a.trade_count == 1);
}

TEST_CASE("total_equity = cash + shares * mark_price", "[chisel][account]") {
  chisel::Account a(10'000.0);
  a.buy(10, 100.0);
  REQUIRE_THAT(a.total_equity(120.0), WithinAbs(10'200.0, 1e-9));
  REQUIRE_THAT(a.total_equity(80.0), WithinAbs(9'800.0, 1e-9));
}

TEST_CASE("buying twice averages the entry price", "[chisel][account]") {
  chisel::Account a(10'000.0);
  a.buy(10, 100.0);
  a.buy(10, 120.0);
  REQUIRE(a.shares == 20);
  REQUIRE_THAT(a.avg_buy_price, WithinAbs(110.0, 1e-9));
}
