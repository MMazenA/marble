#include "engine/account.h"

namespace chisel {

void Account::buy(int quantity, double price) {
  cash -= quantity * price;
  if (shares == 0) {
    avg_buy_price = price;
  } else {
    avg_buy_price =
        (avg_buy_price * shares + price * quantity) / (shares + quantity);
  }
  shares += quantity;
}

void Account::sell(int quantity, double price) {
  cash += quantity * price;
  realized_pnl += (price - avg_buy_price) * quantity;
  shares -= quantity;
  if (shares == 0) {
    avg_buy_price = 0.0;
  }
  ++trade_count;
}

double Account::total_equity(double mark_price) const {
  return cash + (shares * mark_price);
}

} // namespace chisel
