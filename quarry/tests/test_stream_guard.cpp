#include "stream_guard.h"
#include <boost/asio/io_context.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("StreamGuard") {
  SECTION("Stream guard returns the right types") {
    using tcp_stream = boost::beast::tcp_stream;
    net::io_context ioc;
    quarry::StreamGuard stream_guard{ioc};
    REQUIRE(!stream_guard.is_ssl());
  }
}
