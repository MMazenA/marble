#include "aggregates.h"
#include "polygon.h"
#include "utils.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("HTTP Client can send request to endpoint") {
  quarry::load_dotenv("../../quarry/.env");

  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  auto ep = quarry::ep::Aggregates::withTicker("AAPL")
                .fromDate("2024-01-09")
                .toDate("2024-01-11");

  const auto response = polygon.execute(ep);

  REQUIRE(true);
}
