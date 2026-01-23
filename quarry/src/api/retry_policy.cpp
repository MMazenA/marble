#include "retry_policy.h"

#include <chrono>
#include <random>
#include <thread>

namespace quarry {

RetryPolicy::RetryPolicy()
    : m_initial_ms(100), m_max_wait_ms(1000),
      m_strategy(PolicyStrategy::exponential), m_max_attempts(3) {
  m_retry_lookup.set(DEAD_STREAM_ERROR_CODE); // internal code for dead stream
  m_retry_lookup.set(503);
  m_retry_lookup.set(502);
  m_retry_lookup.set(504);
  m_retry_lookup.set(500);

  m_retry_lookup.set(429);
}

bool RetryPolicy::should_retry(unsigned int http_code) const noexcept {
  return http_code < m_retry_lookup.size() && m_retry_lookup.test(http_code);
}

// https://aws.amazon.com/blogs/architecture/exponential-backoff-and-jitter/
int RetryPolicy::get_wait_time(Count_type attempt_n) const noexcept {
  auto strategy_bound = m_get_backoff_time(attempt_n);

  thread_local uint64_t s = [] {
    const uint64_t t = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    uint64_t x = t ^ reinterpret_cast<uintptr_t>(&t);
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x ? x : 0x9e3779b97f4a7c15ULL;
  }();

  struct Xorshift {
    using result_type = uint64_t;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }
    result_type operator()() {
      s ^= s >> 12;
      s ^= s << 25;
      s ^= s >> 27;
      return s * 0x2545F4914F6CDD1DULL;
    }
    uint64_t &s;
  };

  Xorshift rng{s};
  std::uniform_int_distribution<int> dist(
      0, std::min(m_max_wait_ms, strategy_bound));

  return dist(rng);
};

void RetryPolicy::wait(uint8_t attempt_n) const noexcept {
  const int wait_ms = get_wait_time(attempt_n);
  if (wait_ms <= 0) {
    return;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
}

RetryPolicy::Ms_type
RetryPolicy::m_get_backoff_time(Count_type attempt_n) const noexcept {
  switch (m_strategy) {
  case (PolicyStrategy::exponential):
    return m_initial_ms * (1 << attempt_n); // initial_ms * 2^n
  default:
    return m_initial_ms;
  }
}

int RetryPolicy::get_max_attempts() const noexcept { return m_max_attempts; };

} // namespace quarry
