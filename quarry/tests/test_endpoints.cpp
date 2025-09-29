#include "aggregates.h"
#include "polygon.h"
#include "utils.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

TEST_CASE("Endpoint can be made") {
  auto ep = quarry::Endpoint::AggregatesRequest::withTicker("AAPL")
                .fromDate("2024-01-09")
                .toDate("2024-01-11");

  REQUIRE(true);
}

// TEST_CASE("Endpoint can be customized") {
//   auto ep =
//       quarry::Endpoint::AggregatesRequest("AAPL", "2024-01-09",
//       "2024-01-11");

//   ep.setAdjusted(false).setLimit(1).setSort(quarry::sort_options::ASC);
//   REQUIRE(ep.m_adjusted == false);
//   REQUIRE(ep.m_limit == 1);
//   REQUIRE(ep.m_sort == quarry::sort_options::ASC);
// }

// TEST_CASE("Endpoint throws on improper date") {
//   REQUIRE_THROWS_AS(
//       quarry::Endpoint::AggregatesDaily("AAPL", "2024-1-09", "2024-01-11"),
//       std::invalid_argument);
// }

// TEST_CASE("Endpoint throws when start date is past end date") {
//   REQUIRE_THROWS_AS(
//       quarry::Endpoint::AggregatesDaily("AAPL", "2030-01-09", "2024-01-11"),
//       std::invalid_argument);
// }

// TEST_CASE("Endpoint can modify to and from dates") {
//   auto ep = quarry::Endpoint::AggregatesDaily("AAPL");
//   std::string from_date = "2024-01-09";
//   std::string to_date = "2024-01-10";
//   ep.setFromDate(from_date).setToDate(to_date);

//   REQUIRE(ep.m_from_date == from_date);
//   REQUIRE(ep.m_to_date == to_date);
// }
