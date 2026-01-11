#include "api/retry_policy.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <chrono>

using namespace quarry;

TEST_CASE("RetryPolicy Construction") {
  SECTION("Constructs with single retry code") {
    RetryPolicy policy(100, 1000, PolicyStrategy::exponential, 3, 500);
    REQUIRE(policy.should_retry(500));
    REQUIRE_FALSE(policy.should_retry(200));
  }

  SECTION("Constructs with multiple retry codes") {
    RetryPolicy policy(100, 1000, PolicyStrategy::exponential, 5, 429, 500, 502, 503, 504);
    REQUIRE(policy.should_retry(429));
    REQUIRE(policy.should_retry(500));
    REQUIRE(policy.should_retry(502));
    REQUIRE(policy.should_retry(503));
    REQUIRE(policy.should_retry(504));
    REQUIRE_FALSE(policy.should_retry(200));
    REQUIRE_FALSE(policy.should_retry(404));
  }

  SECTION("Constructs with no retry codes") {
    RetryPolicy policy(100, 1000, PolicyStrategy::exponential, 3);
    REQUIRE_FALSE(policy.should_retry(500));
    REQUIRE_FALSE(policy.should_retry(200));
  }
}

TEST_CASE("RetryPolicy should_retry") {
  RetryPolicy policy(100, 1000, PolicyStrategy::exponential, 3, 429, 500, 502, 503);

  SECTION("Returns true for configured retry codes") {
    REQUIRE(policy.should_retry(429));
    REQUIRE(policy.should_retry(500));
    REQUIRE(policy.should_retry(502));
    REQUIRE(policy.should_retry(503));
  }

  SECTION("Returns false for non-retry codes") {
    REQUIRE_FALSE(policy.should_retry(200));
    REQUIRE_FALSE(policy.should_retry(201));
    REQUIRE_FALSE(policy.should_retry(400));
    REQUIRE_FALSE(policy.should_retry(404));
    REQUIRE_FALSE(policy.should_retry(501));
  }

  SECTION("Returns false for negative HTTP codes") {
    REQUIRE_FALSE(policy.should_retry(-1));
    REQUIRE_FALSE(policy.should_retry(-100));
  }

  SECTION("Returns false for HTTP codes >= MAX_HTTP_STATUS_CODE") {
    REQUIRE_FALSE(policy.should_retry(600));
    REQUIRE_FALSE(policy.should_retry(700));
    REQUIRE_FALSE(policy.should_retry(1000));
  }

  SECTION("Handles edge case at boundary") {
    REQUIRE_FALSE(policy.should_retry(599));
  }
}

TEST_CASE("RetryPolicy get_wait_time - Exponential backoff") {
  SECTION("Returns time within bounds for attempt 0") {
    RetryPolicy policy(100, 10000, PolicyStrategy::exponential, 5, 500);
    int wait_time = policy.get_wait_time(0);
    // For attempt 0: initial_ms * 2^0 = 100
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 100);
  }

  SECTION("Returns time within bounds for attempt 1") {
    RetryPolicy policy(100, 10000, PolicyStrategy::exponential, 5, 500);
    int wait_time = policy.get_wait_time(1);
    // For attempt 1: initial_ms * 2^1 = 200
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 200);
  }

  SECTION("Returns time within bounds for attempt 2") {
    RetryPolicy policy(100, 10000, PolicyStrategy::exponential, 5, 500);
    int wait_time = policy.get_wait_time(2);
    // For attempt 2: initial_ms * 2^2 = 400
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 400);
  }

  SECTION("Respects max_wait_ms cap") {
    RetryPolicy policy(100, 500, PolicyStrategy::exponential, 10, 500);
    int wait_time = policy.get_wait_time(5);
    // For attempt 5: initial_ms * 2^5 = 3200, but max_wait_ms = 500
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 500);
  }

  SECTION("Handles large attempt numbers") {
    RetryPolicy policy(100, 5000, PolicyStrategy::exponential, 20, 500);
    int wait_time = policy.get_wait_time(10);
    // For attempt 10: initial_ms * 2^10 = 102400, capped by max_wait_ms
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 5000);
  }

  SECTION("Returns different values with jitter") {
    RetryPolicy policy(100, 10000, PolicyStrategy::exponential, 5, 500);

    constexpr int num_samples = 100;
    int wait_times[num_samples];
    bool has_variation = false;

    for (int i = 0; i < num_samples; ++i) {
      wait_times[i] = policy.get_wait_time(3);
      if (i > 0 && wait_times[i] != wait_times[0]) {
        has_variation = true;
      }
    }

    REQUIRE(has_variation);
  }
}

TEST_CASE("RetryPolicy get_wait_time - Edge cases") {
  SECTION("Handles small initial_ms") {
    RetryPolicy policy(1, 1000, PolicyStrategy::exponential, 5, 500);
    int wait_time = policy.get_wait_time(0);
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 1);
  }

  SECTION("Handles initial_ms equal to max_wait_ms") {
    RetryPolicy policy(1000, 1000, PolicyStrategy::exponential, 5, 500);
    int wait_time = policy.get_wait_time(5);
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 1000);
  }

  SECTION("Handles max_wait_ms smaller than exponential result") {
    RetryPolicy policy(1000, 500, PolicyStrategy::exponential, 5, 500);
    int wait_time = policy.get_wait_time(1);
    // Exponential would be 2000, but max is 500
    REQUIRE(wait_time >= 0);
    REQUIRE(wait_time <= 500);
  }

}
