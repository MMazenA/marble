#include "engine/engine.h"

#include <utility>

namespace chisel {

BacktestResult run_backtest(const std::vector<Bar> &bars, std::string symbol,
                            double starting_cash) {
  Account account(starting_cash);
  MACrossover strategy;
  std::vector<Snapshot> snapshots;
  snapshots.reserve(bars.size());

  for (std::size_t i = 0; i + 1 < bars.size(); ++i) {
    const Bar &bar = bars[i];
    const Bar &next_bar = bars[i + 1];

    Action signal = strategy.on_bar(bar.close);
    std::optional<Trade> trade;

    if (signal == Action::Buy && account.shares == 0) {
      const double price = next_bar.open;
      const int qty = static_cast<int>(account.cash / price);
      if (qty > 0) {
        account.buy(qty, price);
        trade = Trade{.side = Action::Buy, .quantity = qty, .price = price};
      }
    } else if (signal == Action::Sell && account.shares > 0) {
      const double price = next_bar.open;
      const int qty = account.shares;
      account.sell(qty, price);
      trade = Trade{.side = Action::Sell, .quantity = qty, .price = price};
    }

    const double equity = account.total_equity(bar.close);
    snapshots.push_back(Snapshot{.step = i,
                                 .bar = bar,
                                 .signal = signal,
                                 .trade = trade,
                                 .account = account,
                                 .total_equity = equity});
  }

  const double final_equity =
      bars.empty() ? starting_cash : account.total_equity(bars.back().close);

  Summary summary;
  summary.start_balance = starting_cash;
  summary.end_balance = final_equity;
  summary.return_percent =
      starting_cash > 0.0
          ? (final_equity - starting_cash) / starting_cash * 100.0
          : 0.0;
  summary.trade_count = account.trade_count;

  return BacktestResult{.symbol = std::move(symbol),
                        .summary = summary,
                        .snapshots = std::move(snapshots)};
}

} // namespace chisel
