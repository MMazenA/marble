#include "strategy/ma_crossover.h"

#include <numeric>

namespace chisel {

Action MACrossover::on_bar(double close) {
  m_closes.push_back(close);
  if (m_closes.size() > m_slow) {
    m_closes.pop_front();
  }
  if (m_closes.size() < m_slow) {
    return Action::Hold;
  }

  const auto fast_start = m_closes.end() - static_cast<long>(m_fast);
  const double fast_avg =
      std::accumulate(fast_start, m_closes.end(), 0.0) / m_fast;
  const double slow_avg =
      std::accumulate(m_closes.begin(), m_closes.end(), 0.0) / m_slow;

  const bool was_fast_above = m_fast_above;
  m_fast_above = fast_avg > slow_avg;

  if (!m_seeded) {
    m_seeded = true;
    return Action::Hold;
  }
  if (!was_fast_above && m_fast_above) {
    return Action::Buy;
  }
  if (was_fast_above && !m_fast_above) {
    return Action::Sell;
  }
  return Action::Hold;
}

} // namespace chisel
