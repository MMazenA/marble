#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <utility>

#include <grpcpp/grpcpp.h>

#include "back_test.grpc.pb.h"
#include "back_test.pb.h"
#include "logging.h"

namespace {

struct Args {
  std::string target = "localhost:50052";
  std::string symbol = "AAPL";
  std::string from_date;
  std::string to_date;
  double cash = 100'000.0;
};

void print_help() {
  std::println("usage: chisel_client [options]");
  std::println("  -s, --symbol SYMBOL   ticker (default: AAPL)");
  std::println("  --from DATE           start date YYYY-MM-DD");
  std::println("  --to   DATE           end date   YYYY-MM-DD");
  std::println("  --cash AMOUNT         starting cash (default: 100000)");
  std::println("  --target HOST:PORT    server (default: localhost:50052)");
  std::println("  -h, --help            show this help");
}

std::int64_t iso_to_epoch_ms(const std::string &date) {
  using namespace std::chrono;
  const int y = std::stoi(date.substr(0, 4));
  const int m = std::stoi(date.substr(5, 2));
  const int d = std::stoi(date.substr(8, 2));
  const auto ymd =
      year{y} / month{static_cast<unsigned>(m)} / day{static_cast<unsigned>(d)};
  return duration_cast<milliseconds>(sys_days{ymd}.time_since_epoch()).count();
}

Args parse_args(int argc, char **argv) {
  Args args;
  for (int i = 1; i < argc; ++i) {
    std::string_view a = argv[i];
    if (a == "-h" || a == "--help") {
      print_help();
      std::exit(0);
    } else if (a == "-s" || a == "--symbol") {
      args.symbol = argv[++i];
    } else if (a == "--from") {
      args.from_date = argv[++i];
    } else if (a == "--to") {
      args.to_date = argv[++i];
    } else if (a == "--cash") {
      args.cash = std::stod(argv[++i]);
    } else if (a == "--target") {
      args.target = argv[++i];
    }
  }
  return args;
}

const char *action_label(marble::ActionType a) {
  switch (a) {
  case marble::ACTION_BUY:
    return "BUY";
  case marble::ACTION_SELL:
    return "SELL";
  default:
    return "HOLD";
  }
}

void print_event(const marble::BacktestEvent &ev) {
  switch (ev.event_case()) {
  case marble::BacktestEvent::kStarted: {
    const auto &cfg = ev.started();
    std::println("[started] strategy={} symbol={} start={} end={}",
                 cfg.strategy_name(), cfg.symbol(), cfg.start(), cfg.end());
    break;
  }
  case marble::BacktestEvent::kStep: {
    const auto &snap = ev.step();
    const auto &acct = snap.account();
    std::string trade_str = "-";
    if (snap.has_trade()) {
      const auto &t = snap.trade();
      trade_str = std::format("{}@{:.2f} qty={}", action_label(t.action()),
                              t.price(), t.quantity());
    }
    std::println("[step={:>3}] close={:.2f} sig={:<4} trade={:<24} cash={:.2f} "
                 "shares={} equity={:.2f}",
                 snap.step(), snap.candle().close(),
                 action_label(snap.signal().action()), trade_str, acct.cash(),
                 acct.shares(), acct.total_equity());
    break;
  }
  case marble::BacktestEvent::kEnd: {
    const auto &s = ev.end();
    std::println("==== summary ====");
    std::println("start_balance: {:.2f}", s.start_balance());
    std::println("end_balance:   {:.2f}", s.end_balance());
    std::println("return:        {:.2f}%", s.return_percent());
    std::println("trades:        {}", s.trade_count());
    break;
  }
  case marble::BacktestEvent::EVENT_NOT_SET:
    break;
  }
}

} // namespace

int main(int argc, char **argv) {
  using namespace std::chrono;

  const Args args = parse_args(argc, argv);

  marble::BacktestRequest req;
  req.set_symbol(args.symbol);
  req.set_starting_cash(args.cash);
  if (!args.from_date.empty()) {
    req.set_start(iso_to_epoch_ms(args.from_date));
  }
  if (!args.to_date.empty()) {
    req.set_end(iso_to_epoch_ms(args.to_date));
  }

  auto channel =
      grpc::CreateChannel(args.target, grpc::InsecureChannelCredentials());
  auto stub = marble::BacktestService::NewStub(std::move(channel));

  grpc::ClientContext ctx;
  auto reader = stub->RunBackTest(&ctx, req);

  const auto rpc_start = steady_clock::now();
  marble::BacktestEvent ev;
  std::size_t event_count = 0;
  while (reader->Read(&ev)) {
    print_event(ev);
    ++event_count;
  }
  const grpc::Status status = reader->Finish();
  const auto rpc_end = steady_clock::now();

  if (!status.ok()) {
    auto *logger = quarry::logging::init();
    LOG_ERROR(logger, "RunBackTest failed: {}", status.error_message());
    return 1;
  }

  std::println("---");
  std::println("received {} events in {}ms", event_count,
               duration_cast<microseconds>(rpc_end - rpc_start).count() /
                   1000.0);
  return 0;
}
