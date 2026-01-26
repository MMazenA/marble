#ifndef CHISEL_STRATEGY_MA_CROSSOVER_H
#define CHISEL_STRATEGY_MA_CROSSOVER_H

#include "strategy/i_strategy.h"

namespace chisel {

class MACrossover : public IStrategy {
public:
  MACrossover();
  ~MACrossover() override;
};

} // namespace chisel

#endif // CHISEL_STRATEGY_MA_CROSSOVER_H
