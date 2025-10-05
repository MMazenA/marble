#include "aggregates.h"
#include "polygon.h"
#include "utils.h"
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>
#include <iostream>

TEST_CASE("Polygon can send request to endpoint") {
  quarry::load_dotenv("../../quarry/.env");

  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  auto ep = quarry::ep::Aggregates::withTicker("AAPL")
                .fromDate("2024-01-09")
                .toDate("2024-01-11");

  quarry::ep::Aggregates::response_type response = polygon.execute(ep);

  REQUIRE(response.ticker == "AAPL");
  REQUIRE(response.resultsCount == 3);

  REQUIRE(true);
}
