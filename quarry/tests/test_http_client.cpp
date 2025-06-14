#include "http_client.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("HTTP Client can connect") {
  // MigrationMock migrationMock;
  http::response<http::dynamic_body> http_response;
  quarry::client("www.example.com", "80", "/", http_response);

  REQUIRE(static_cast<int>(http_response.result()) == 200);
}
