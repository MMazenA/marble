#include "api/http_request_builder.h"
#include "api/transport_pool.h"
#include "dns_cache.h"
#include "http_types.h"
#include "ssl_context_provider.h"

#include "transport.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/verb.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <thread>
#include <vector>

TEST_CASE("TransportPool") {
  using namespace std::chrono;
  using duration_type =
      std::chrono::duration<long long, std::ratio<1, 1000000>>;

  constexpr const char *test_host = "localhost";
  // NOLINTNEXTLINE
  constexpr uint16_t test_port_http = 18080;
  // NOLINTNEXTLINE
  constexpr uint16_t test_port_https = 18443;

  SECTION("Pool is faster on threaded requests") {
    uint16_t max_conn = 2;
    net::io_context ioc;
    quarry::DnsCacheContext context{
        .host = test_host,
        .ioc = ioc,
        .port = test_port_https,
        .is_tls = true,
    };
    const auto endpoints = quarry::DnsCache::global_cache().get(context);
    auto ssl_ctx = quarry::SslContextProvider::make_insecure_client_ctx();

    quarry::TransportPool pool{max_conn, std::string{context.host}, ioc,
                               ssl_ctx, endpoints};

    auto req = quarry::HttpRequestBuilder{}
                   .verb(boost::beast::http::verb::get)
                   .target("/get")
                   .version(11)
                   .host(context.host)
                   .user_agent(BOOST_BEAST_VERSION_STRING)
                   .headers({})
                   .keep_alive(true)
                   .build();

    duration_type single_duration;
    duration_type pool_duration;

    const auto requests_count = 3;

    // single transport
    {
      auto start = high_resolution_clock::now();
      for (int i = 0; i < requests_count; ++i) {
        quarry::Transport transport(std::string{context.host}, ioc, ssl_ctx);
        transport.connect(endpoints);
        http::response<http::string_body> resp;
        transport.write(req);
        transport.read(resp);
        REQUIRE(resp.result_int() == 200);
      }
      auto stop = high_resolution_clock::now();
      single_duration = duration_cast<microseconds>(stop - start);
    }

    // pooled transport
    {
      std::vector<http::response<http::string_body>> responses(requests_count);
      std::vector<std::thread> threads;
      threads.reserve(requests_count);

      auto start = high_resolution_clock::now();
      for (int i = 0; i < requests_count; ++i) {
        threads.emplace_back(
            [&, i]() { pool.send_and_read(req, responses[i]); });
      }
      for (auto &t : threads) {
        t.join();
      }
      auto stop = high_resolution_clock::now();
      pool_duration = duration_cast<microseconds>(stop - start);

      for (int i = 0; i < requests_count; ++i) {
        REQUIRE(responses[i].result_int() == 200);
      }
    }

    std::println("single (3 fresh connects): {}", single_duration);
    std::println("pool (3 threaded, pre-connected): {}", pool_duration);

    REQUIRE(single_duration > pool_duration);
  }

  SECTION("Can reuse stream for multiple threaded requests") {
    net::io_context ioc;
    quarry::DnsCacheContext context{
        .host = test_host,
        .ioc = ioc,
        .port = test_port_https,
        .is_tls = true,
    };
    const auto endpoints = quarry::DnsCache::global_cache().get(context);
    auto ssl_ctx = quarry::SslContextProvider::make_insecure_client_ctx();

    quarry::TransportPool pool{1, std::string{context.host}, ioc, ssl_ctx,
                               endpoints};

    auto req = quarry::HttpRequestBuilder{}
                   .verb(boost::beast::http::verb::get)
                   .target("/get")
                   .version(11)
                   .host(context.host)
                   .user_agent(BOOST_BEAST_VERSION_STRING)
                   .headers({})
                   .keep_alive(true)
                   .build();

    const int num_requests = 5;
    std::vector<http::response<http::string_body>> responses(num_requests);
    std::vector<std::thread> threads;
    threads.reserve(num_requests);

    for (int i = 0; i < num_requests; ++i) {
      threads.emplace_back([&, i]() {
        pool.send_and_read(req, responses[i]);
        INFO("Thread " << i + 1 << ": status " << responses[i].result_int());
      });
    }

    for (auto &t : threads) {
      t.join();
    }

    for (int i = 0; i < num_requests; ++i) {
      REQUIRE(responses[i].result_int() == 200);
    }
  }

  SECTION("Pool is faster on sequential requests") {
    uint16_t max_conn = 1;
    net::io_context ioc;
    quarry::DnsCacheContext context{
        .host = test_host,
        .ioc = ioc,
        .port = test_port_https,
        .is_tls = true,
    };
    const auto endpoints = quarry::DnsCache::global_cache().get(context);
    auto ssl_ctx = quarry::SslContextProvider::make_insecure_client_ctx();

    auto req = quarry::HttpRequestBuilder{}
                   .verb(boost::beast::http::verb::get)
                   .target("/get")
                   .version(11)
                   .host(context.host)
                   .user_agent(BOOST_BEAST_VERSION_STRING)
                   .headers({})
                   .keep_alive(true)
                   .build();

    quarry::TransportPool pool{max_conn, std::string{context.host}, ioc,
                               ssl_ctx, endpoints};
    quarry::Transport single_transport(std::string{context.host}, ioc, ssl_ctx);

    duration_type single_duration;
    duration_type pool_duration;

    // Single
    {
      auto start = high_resolution_clock::now();
      single_transport.connect(endpoints);
      http::response<http::string_body> resp;
      single_transport.write(req);
      single_transport.read(resp);
      auto stop = high_resolution_clock::now();

      single_duration = duration_cast<microseconds>(stop - start);
      REQUIRE(resp.result_int() == 200);
    }

    // Pool
    {
      auto start = high_resolution_clock::now();
      http::response<http::string_body> resp1;
      http::response<http::string_body> resp2;

      pool.send_and_read(req, resp1);
      pool.send_and_read(req, resp2);

      auto stop = high_resolution_clock::now();

      pool_duration = duration_cast<microseconds>(stop - start);
      REQUIRE(resp1.result_int() == 200);
      REQUIRE(resp2.result_int() == 200);
    }

    std::println("single (connect+handshake+1req): {}", single_duration);
    std::println("pool (2 reqs, no handshake): {}", pool_duration);

    REQUIRE(single_duration > pool_duration);
  }
}
