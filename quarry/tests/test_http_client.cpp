#include "http_client.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("HttpClient") {
  SECTION("HTTP Client can connect to an endpoint") {
    quarry::HttpClient client("httpbin.org", 80);
    const auto response = client.get("/");
    REQUIRE(response.result_int() == 200);
  }

  SECTION("HTTP Client can connect to an endpoint and target") {
    quarry::HttpClient client("httpbin.org", 80);
    const auto response = client.get("/user-agent");
    REQUIRE(response.result_int() == 200);
  }

  SECTION("HTTPS Client can connect to an endpoint") {
    quarry::HttpClient client("httpbin.org", 443);
    const auto response = client.get("/");
    REQUIRE(response.result_int() == 200);
  }

  SECTION("HTTPS Client can connect to an endpoint and target") {
    quarry::HttpClient client("httpbin.org", 443);
    const auto response = client.get("/user-agent");
    REQUIRE(response.result_int() == 200);
  }
}
