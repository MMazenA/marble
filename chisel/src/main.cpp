#include <cstdlib>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "api/endpoints/aggregates.h"
#include "api/massive.h"
#include "engine/engine.h"
#include "logging.h"
#include "utils.h"

namespace {

const char *action_label(chisel::Action a) {
  switch (a) {
  case chisel::Action::Buy:
    return "BUY";
  case chisel::Action::Sell:
    return "SELL";
  case chisel::Action::Hold:
    return "HOLD";
  }
  return "?";
}

void print_snapshot(const chisel::Snapshot &s) {
  std::string trade_str = "-";
  if (s.trade.has_value()) {
    trade_str = std::format("{}@{:.2f} qty={}", action_label(s.trade->side),
                            s.trade->price, s.trade->quantity);
  }
  std::println("step={:>3} close={:.2f} sig={:<4} trade={:<24} cash={:.2f} "
               "shares={} equity={:.2f}",
               s.step, s.bar.close, action_label(s.signal), trade_str,
               s.account.cash, s.account.shares, s.total_equity);
}

void print_summary(const chisel::Summary &s) {
  std::println("==== summary ====");
  std::println("start_balance: {:.2f}", s.start_balance);
  std::println("end_balance:   {:.2f}", s.end_balance);
  std::println("return:        {:.2f}%", s.return_percent);
  std::println("trades:        {}", s.trade_count);
}

} // namespace

int main() {
  quarry::logging::Config log_config{};
  log_config.logger_name = "chisel";
  log_config.log_path = "logs/chisel.log";
  auto *logger = quarry::logging::init(log_config);
  quarry::load_dotenv();

  const char *api_key = std::getenv("MASSIVE_API_KEY");
  if ((api_key == nullptr) || std::string_view(api_key).empty()) {
    LOG_ERROR(logger, "MASSIVE_API_KEY is not set");
    return 1;
  }

  quarry::Massive massive(api_key);

  const std::string symbol = "AAPL";
  auto ep = quarry::ep::Aggregates::with_ticker(symbol)
                .time_span(quarry::timespan_options::DAY)
                .from_date("2024-01-01")
                .to_date("2025-01-01")
                .limit(360);

  std::vector<chisel::Bar> bars;
  try {
    const auto response = massive.execute(ep);
    if (response.results.has_value()) {
      bars.reserve(response.results->size());
      for (const auto &b : *response.results) {
        bars.push_back({b.o, b.c, b.t});
      }
    }
  } catch (const std::exception &ex) {
    LOG_ERROR(logger, "Massive request failed: {}", ex.what());
    return 1;
  }

  LOG_INFO(logger, "Fetched {} bars", bars.size());
  if (bars.size() < 60) {
    LOG_ERROR(logger, "Need at least 60 bars for MA(20/50), got {}",
              bars.size());
    return 1;
  }

  auto result = chisel::run_backtest(bars, symbol, 100'000.0);

  for (const auto &snap : result.snapshots) {
    print_snapshot(snap);
  }
  print_summary(result.summary);

  return 0;
}
