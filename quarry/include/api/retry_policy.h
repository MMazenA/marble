#ifndef QUARRY_RETRY_POLICY
#define QUARRY_RETRY_POLICY

#include <bitset>
#include <cmath>
#include <concepts>
#include <random>
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
    int strategy_bound = m_get_fall_back_time(attempt_n);

    static std::mt19937 rng{std::random_device{}()};

    std::uniform_int_distribution<int> dist(
        0, std::min(m_max_wait_ms, strategy_bound));

    return dist(rng);
  };

private:
  [[nodiscard]] int m_get_fall_back_time(uint8_t attempt_n) const {
    switch (m_strategy) {
    case (PolicyStrategy::exponential):
      return m_initial_ms * static_cast<int>(pow(2, attempt_n));
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
