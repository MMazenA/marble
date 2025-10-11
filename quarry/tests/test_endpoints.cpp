#include "aggregates.h"
#include "polygon.h"
#include "utils.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

TEST_CASE("Endpoint can be made") {
  auto ep = quarry::ep::Aggregates::withTicker("AAPL")
                .fromDate("2024-01-09")
                .toDate("2024-01-11");

  REQUIRE(true);
}

TEST_CASE("Endpoint can be customized") {
  auto ep = quarry::ep::Aggregates::withTicker("AAPL")
                .fromDate("2024-01-09")
                .toDate("2024-01-11")
                .adjusted(false)
                .limit(1)
                .sort(quarry::sort_options::ASC);

  REQUIRE(ep.m_adjusted == false);
  REQUIRE(ep.m_limit == 1);
  REQUIRE(ep.m_sort == quarry::sort_options::ASC);
}

TEST_CASE("Endpoint throws on improper date") {
  REQUIRE_THROWS_AS(quarry::ep::Aggregates::withTicker("AAPL")
                        .toDate("2024-1-09")
                        .fromDate("2024-01-11"),
                    std::invalid_argument);
}
