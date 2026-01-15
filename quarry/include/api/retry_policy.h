#ifndef QUARRY_RETRY_POLICY
#define QUARRY_RETRY_POLICY

#include <bitset>
#include <concepts>
namespace quarry {

// NOLINTNEXTLINE
static constexpr int DEAD_STREAM_ERROR_CODE = 0;

enum class PolicyStrategy : std::uint8_t {
  exponential,
};

class RetryPolicy {
public:
  RetryPolicy();

  template <std::integral... codes>
  RetryPolicy(int initial_ms, int max_wait_ms, PolicyStrategy strategy,
              uint8_t max_attempts, codes... retry_on_codes)
      : m_initial_ms(initial_ms), m_max_wait_ms(max_wait_ms),
        m_strategy(strategy), m_max_attempts(max_attempts) {
    m_retry_lookup.set(DEAD_STREAM_ERROR_CODE); // internal code for dead stream
    (m_retry_lookup.set(static_cast<int>(retry_on_codes)), ...);
  }

  [[nodiscard]] bool should_retry(unsigned int http_code) const noexcept;

  // https://aws.amazon.com/blogs/architecture/exponential-backoff-and-jitter/
  [[nodiscard]] int get_wait_time(uint8_t attempt_n) const;

  [[nodiscard]] int get_max_attempts() const;

  void wait(uint8_t attempt_n) const noexcept;

private:
  [[nodiscard]] int m_get_backoff_time(uint8_t attempt_n) const;

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
