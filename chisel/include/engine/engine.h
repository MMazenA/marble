#ifndef CHISEL_ENGINE_ENGINE_H
#define CHISEL_ENGINE_ENGINE_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "engine/account.h"
#include "strategy/ma_crossover.h"

namespace chisel {

struct Bar {
  double open;
  double close;
  std::int64_t timestamp = 0;
};

struct Trade {
  Action side;
  int quantity;
  double price;
};

struct Snapshot {
  std::size_t step;
  Bar bar;
  Action signal;
  std::optional<Trade> trade;
  Account account;
  double total_equity;
};

struct Summary {
  double start_balance;
  double end_balance;
  double return_percent;
  int trade_count;
};

struct BacktestResult {
  std::string symbol;
  Summary summary;
  std::vector<Snapshot> snapshots;
};

BacktestResult run_backtest(const std::vector<Bar> &bars, std::string symbol,
                            double starting_cash);

} // namespace chisel

#endif // CHISEL_ENGINE_ENGINE_H
