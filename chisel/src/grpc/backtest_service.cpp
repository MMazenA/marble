#include "grpc/backtest_service.h"
#include "engine/engine.h"

namespace chisel {

namespace {

marble::ActionType to_proto_action(Action a) {
  switch (a) {
  case Action::Buy:
    return marble::ACTION_BUY;
  case Action::Sell:
    return marble::ACTION_SELL;
  case Action::Hold:
    return marble::ACTION_HOLD;
  }
  return marble::ACTION_HOLD;
}

void fill_account(marble::AccountState &out, const Account &acct,
                  double mark_price) {
  out.set_cash(acct.cash);
  out.set_shares(acct.shares);
  out.set_avg_buy_price(acct.avg_buy_price);
  out.set_realized_pnl(acct.realized_pnl);
  out.set_total_equity(acct.total_equity(mark_price));
}

marble::BacktestEvent make_started_event(const BacktestResult &res,
                                         const marble::BacktestRequest &req) {
  marble::BacktestEvent ev;
  auto *cfg = ev.mutable_started();
  cfg->set_strategy_name("ma_crossover_20_50");
  cfg->set_symbol(res.symbol);
  cfg->set_start(req.start());
  cfg->set_end(req.end());
  return ev;
}

marble::BacktestEvent snapshot_to_event(const Snapshot &s) {
  marble::BacktestEvent ev;
  auto *snap = ev.mutable_step();
  snap->set_step(s.step);

  auto *candle = snap->mutable_candle();
  candle->set_open(s.bar.open);
  candle->set_close(s.bar.close);
  candle->set_t(s.bar.timestamp);

  snap->mutable_signal()->set_action(to_proto_action(s.signal));

  if (s.trade.has_value()) {
    auto *t = snap->mutable_trade();
    t->set_action(to_proto_action(s.trade->side));
    t->set_quantity(s.trade->quantity);
    t->set_price(s.trade->price);
  }

  fill_account(*snap->mutable_account(), s.account, s.bar.close);
  return ev;
}

marble::BacktestEvent summary_to_event(const Summary &s) {
  marble::BacktestEvent ev;
  auto *end = ev.mutable_end();
  end->set_start_balance(s.start_balance);
  end->set_end_balance(s.end_balance);
  end->set_return_percent(s.return_percent);
  end->set_trade_count(s.trade_count);
  return ev;
}

} // namespace

grpc::Status BacktestServiceImpl::RunBackTest(
    grpc::ServerContext *ctx, const marble::BacktestRequest *req,
    grpc::ServerWriter<marble::BacktestEvent> *writer) {
  if (!m_provider) {
    return {grpc::StatusCode::FAILED_PRECONDITION,
            "no bar provider configured"};
  }

  std::vector<Bar> bars;
  try {
    bars = m_provider(*req);
  } catch (const std::exception &ex) {
    return {grpc::StatusCode::INTERNAL, ex.what()};
  }

  const std::string symbol = req->symbol().empty() ? "AAPL" : req->symbol();
  const double starting_cash =
      req->starting_cash() > 0.0 ? req->starting_cash() : 100'000.0;

  auto result = run_backtest(bars, symbol, starting_cash);

  if (ctx->IsCancelled()) {
    return grpc::Status::CANCELLED;
  }
  if (!writer->Write(make_started_event(result, *req))) {
    return grpc::Status::CANCELLED;
  }
  for (const auto &snap : result.snapshots) {
    if (ctx->IsCancelled()) {
      return grpc::Status::CANCELLED;
    }
    if (!writer->Write(snapshot_to_event(snap))) {
      return grpc::Status::CANCELLED;
    }
  }
  if (!writer->Write(summary_to_event(result.summary))) {
    return grpc::Status::CANCELLED;
  }

  return grpc::Status::OK;
}

} // namespace chisel
