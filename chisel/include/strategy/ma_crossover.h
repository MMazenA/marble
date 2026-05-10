#ifndef CHISEL_STRATEGY_MA_CROSSOVER_H
#define CHISEL_STRATEGY_MA_CROSSOVER_H

#include <cstddef>
#include <deque>

namespace chisel {

enum class Action { Hold, Buy, Sell };

class MACrossover {
public:
  MACrossover() = default;
  MACrossover(std::size_t fast, std::size_t slow) : m_fast(fast), m_slow(slow) {}

  Action on_bar(double close);

private:
  std::size_t m_fast = 20;
  std::size_t m_slow = 50;
  std::deque<double> m_closes;
  bool m_seeded = false;
  bool m_fast_above = false;
};

} // namespace chisel

#endif // CHISEL_STRATEGY_MA_CROSSOVER_H
