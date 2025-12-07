#include "dns_cache.h"
#include "http_types.h"
#include <catch2/catch_test_macros.hpp>
#include <print>

TEST_CASE("DnsCache") {
  SECTION("Cached results are faster") {
    using namespace std::chrono;
    using duration_type =
        std::chrono::duration<long long, std::ratio<1, 1000000>>;

    quarry::DnsCache cache;

    net::io_context ioc;
    quarry::DnsCacheContext context{
        .host = "httpbin.org", .ioc = ioc, .port = 80, .is_tls = false};

    duration_type cached_duration;
    duration_type non_cached_duration;

    {
      auto start = high_resolution_clock::now();
      const auto &endpoints_first = cache.get(context);
      auto stop = high_resolution_clock::now();
      REQUIRE(endpoints_first.begin() !=
              endpoints_first.end()); // ensure not empty
      non_cached_duration = duration_cast<microseconds>(stop - start);
    }
    {
      auto start = high_resolution_clock::now();
      const auto &endpoints_second = cache.get(context);
      auto stop = high_resolution_clock::now();
      REQUIRE(endpoints_second.begin() != endpoints_second.end());
      cached_duration = duration_cast<microseconds>(stop - start);
    }

    std::println("Non-cached {}", non_cached_duration);
    std::println("Cached {}", cached_duration);

    REQUIRE(non_cached_duration > cached_duration);
  }
}
