#include "aggregates.h"
#include "massive.h"
#include "utils.h"
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

// Mock JSON response fixture
static constexpr std::string_view MOCK_AGGREGATES_RESPONSE = R"({
  "status": "OK",
  "ticker": "AAPL",
  "adjusted": true,
  "queryCount": 1,
  "resultsCount": 3,
  "count": 3,
  "request_id": "test-request-id",
  "results": [
    {
      "o": 192.51,
      "c": 193.8,
      "h": 194.04,
      "l": 192.34,
      "n": 50000,
      "otc": false,
      "t": 1704844800000,
      "v": 51234000,
      "vw": 193.45
    },
    {
      "o": 193.9,
      "c": 195.1,
      "h": 195.67,
      "l": 193.52,
      "n": 48500,
      "otc": false,
      "t": 1704931200000,
      "v": 49234000,
      "vw": 194.23
    },
    {
      "o": 195.0,
      "c": 194.5,
      "h": 196.1,
      "l": 194.21,
      "n": 52300,
      "otc": false,
      "t": 1705017600000,
      "v": 50234000,
      "vw": 194.89
    }
  ]
})";

TEST_CASE("Massive") {
  SECTION("Parse aggregates response from JSON") {
    auto parsed =
        glz::read_json<quarry::ep::AggregatesR>(MOCK_AGGREGATES_RESPONSE);

    REQUIRE(parsed);
    REQUIRE(parsed->ticker == "AAPL");
    REQUIRE(parsed->resultsCount == 3);
    REQUIRE(parsed->status == "OK");
    REQUIRE(parsed->adjusted == true);
    REQUIRE(parsed->results.has_value());
    REQUIRE(parsed->results->size() == 3);
  }

  SECTION("Aggregates builder constructs valid endpoint") {
    auto ep = quarry::ep::Aggregates::with_ticker("AAPL")
                  .from_date("2024-01-09")
                  .to_date("2024-01-11");

    REQUIRE(ep.m_ticker == "AAPL");
    REQUIRE(ep.m_from_date == "2024-01-09");
    REQUIRE(ep.m_to_date == "2024-01-11");
    REQUIRE(ep.method() == quarry::method_type::GET);
  }

  SECTION("Aggregates builder throws on empty ticker") {
    REQUIRE_THROWS_AS(quarry::ep::Aggregates::with_ticker(""),
                      std::invalid_argument);
  }

  // Uses tokens:

  // SECTION("Massive can send request to endpoint (requires API token)") {
  //   quarry::load_dotenv("../../quarry/.env");
  //   const char *api_key = std::getenv("POLYGON_API_KEY");

  //   if (!api_key) {
  //     SKIP("POLYGON_API_KEY not set");
  //   }

  //   quarry::Massive massive(api_key);

  //   auto ep = quarry::ep::Aggregates::with_ticker("AAPL")
  //                 .from_date("2024-01-09")
  //                 .to_date("2024-01-11");

  //   quarry::ep::Aggregates::response_type response = massive.execute(ep);

  //   REQUIRE(response.ticker == "AAPL");
  //   REQUIRE(response.resultsCount == 3);
  // }
}
