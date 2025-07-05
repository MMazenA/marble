#include "aggregates_daily.h"
#include "polygon.h"
#include "utils.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("HTTP Client can connect") {
  quarry::load_dotenv("../.env");

  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  auto ep =
      quarry::Endpoint::AggregatesDaily("AAPL", "2024-01-09", "2024-01-11");
  const auto response = polygon.execute(ep);

  REQUIRE(true);
}
