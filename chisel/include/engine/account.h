#ifndef CHISEL_ENGINE_ACCOUNT_H
#define CHISEL_ENGINE_ACCOUNT_H

namespace chisel {

struct Account {
  double cash;
  int shares = 0;
  double avg_buy_price = 0.0;
  double realized_pnl = 0.0;
  int trade_count = 0;

  explicit Account(double starting_cash) : cash(starting_cash) {}

  void buy(int quantity, double price);
  void sell(int quantity, double price);

  [[nodiscard]] double total_equity(double mark_price) const;
};

} // namespace chisel

#endif // CHISEL_ENGINE_ACCOUNT_H
