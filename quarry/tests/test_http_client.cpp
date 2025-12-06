#include "http_client.h"
#include "ssl_context_provider.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("HttpClient") {
  constexpr const char *test_host = "localhost";
  // NOLINTNEXTLINE
  constexpr uint16_t test_port_http = 18080;
  // NOLINTNEXTLINE
  constexpr uint16_t test_port_https = 18443;

  SECTION("HTTP Client can connect to an endpoint") {
    quarry::HttpClient client(test_host, test_port_http);
    const auto response = client.get("/");
    REQUIRE(response.result_int() == 200);
  }

  SECTION("HTTP Client can connect to an endpoint and target") {
    quarry::HttpClient client(test_host, test_port_http);
    const auto response = client.get("/get");
    REQUIRE(response.result_int() == 200);
  }

  SECTION("HTTPS Client can connect to an endpoint") {
    quarry::HttpClient client(
        test_host, test_port_https, true,
        &quarry::SslContextProvider::make_insecure_client_ctx);
    const auto response = client.get("/");
    REQUIRE(response.result_int() == 200);
  }

  SECTION("HTTPS Client can connect to an endpoint and target") {
    quarry::HttpClient client(
        test_host, test_port_https, true,
        &quarry::SslContextProvider::make_insecure_client_ctx);
    const auto response = client.get("/get");
    REQUIRE(response.result_int() == 200);
  }
}
