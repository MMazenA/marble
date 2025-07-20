#include "http_client.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("HTTP Client can connect to an endpoint") {
  quarry::httpClient client("httpbin.org", "80");

  const auto response = client.get("/");

  REQUIRE(response.result_int() == 200);
}

TEST_CASE("HTTP Client can connect to an endpoint and target") {
  quarry::httpClient client("httpbin.org", "80");

  const auto response = client.get("/user-agent");

  REQUIRE(response.result_int() == 200);
}

TEST_CASE("HTTPS Client can connect to an endpoint") {
  quarry::httpClient client("httpbin.org", "443");

  const auto response = client.get("/");

  REQUIRE(response.result_int() == 200);
}

TEST_CASE("HTTPS Client can connect to an endpoint and target") {
  quarry::httpClient client("httpbin.org", "443");

  const auto response = client.get("/user-agent");

  REQUIRE(response.result_int() == 200);
}
