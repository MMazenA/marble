#ifndef QUARRY_RETRY_POLICY
#define QUARRY_RETRY_POLICY

#include <bitset>
#include <concepts>
#include <random>
#include <chrono>
namespace quarry {

enum class PolicyStrategy : std::uint8_t {
  exponential,
};

class RetryPolicy {
public:
  template <std::integral... codes>
  RetryPolicy(int initial_ms, int max_wait_ms, PolicyStrategy strategy,
              uint8_t max_attempts, codes... retry_on_codes)
      : m_initial_ms(initial_ms), m_max_wait_ms(max_wait_ms),
        m_strategy(strategy), m_max_attempts(max_attempts) {

    (m_retry_lookup.set(static_cast<int>(retry_on_codes)), ...);
  }

  [[nodiscard]] bool should_retry(int http_code) const {
    return http_code >= 0 && http_code < m_retry_lookup.size() &&
           m_retry_lookup.test(http_code);
  }

  // https://aws.amazon.com/blogs/architecture/exponential-backoff-and-jitter/
  [[nodiscard]] int get_wait_time(uint8_t attempt_n) const {
    int strategy_bound = m_get_backoff_time(attempt_n);

    thread_local uint64_t s = [] {
        const uint64_t t = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        );
        uint64_t x = t ^ reinterpret_cast<uintptr_t>(&t);
        x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
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
      uint64_t& s;
    };

    Xorshift rng{s};
    std::uniform_int_distribution<int> dist(
        0, std::min(m_max_wait_ms, strategy_bound));

    return dist(rng);
  };

private:
  [[nodiscard]] int m_get_backoff_time (uint8_t attempt_n) const {
    switch (m_strategy) {
    case (PolicyStrategy::exponential):
      return m_initial_ms * (1 << attempt_n); // initial_ms * 2^n
    default:
      return m_initial_ms;
    }
  }

  // NOLINTNEXTLINE
  static constexpr int MAX_HTTP_STATUS_CODE = 600;

  int m_initial_ms;
  int m_max_wait_ms;
  PolicyStrategy m_strategy;
  uint8_t m_max_attempts;
  std::bitset<MAX_HTTP_STATUS_CODE> m_retry_lookup;
};

} // namespace quarry

#endif
