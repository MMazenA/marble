#include <chrono>
#include <cstdlib>
#include <format>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "api/endpoints/aggregates.h"
#include "api/massive.h"
#include "engine/engine.h"
#include "grpc/backtest_service.h"
#include "logging.h"
#include "utils.h"

namespace {

constexpr std::string_view k_listen_addr = "0.0.0.0:50052";

std::string epoch_ms_to_iso(std::int64_t epoch_ms) {
  using namespace std::chrono;
  const sys_seconds tp{seconds{epoch_ms / 1000}};
  return std::format("{:%Y-%m-%d}", tp);
}

} // namespace

int main() {
  quarry::logging::Config log_config{};
  log_config.logger_name = "chisel_server";
  log_config.log_path = "logs/chisel_server.log";
  auto *logger = quarry::logging::init(log_config);
  quarry::load_dotenv();

  const char *api_key = std::getenv("MASSIVE_API_KEY");
  if ((api_key == nullptr) || std::string_view(api_key).empty()) {
    LOG_ERROR(logger, "MASSIVE_API_KEY is not set");
    return 1;
  }

  auto massive = std::make_shared<quarry::Massive>(api_key);

  auto provider =
      [massive,
       logger](const marble::BacktestRequest &req) -> std::vector<chisel::Bar> {
    const std::string symbol = req.symbol().empty() ? "AAPL" : req.symbol();
    const std::string from_date =
        req.start() > 0 ? epoch_ms_to_iso(req.start()) : "2024-01-01";
    const std::string to_date =
        req.end() > 0 ? epoch_ms_to_iso(req.end()) : "2025-01-01";

    auto ep = quarry::ep::Aggregates::with_ticker(symbol)
                  .time_span(quarry::timespan_options::DAY)
                  .from_date(from_date)
                  .to_date(to_date)
                  .limit(1500);

    std::vector<chisel::Bar> bars;
    const auto response = massive->execute(ep);
    if (response.results.has_value()) {
      bars.reserve(response.results->size());
      for (const auto &b : *response.results) {
        bars.push_back({b.o, b.c, b.t});
      }
    }
    LOG_INFO(logger, "fetched {} bars for {} {}..{}", bars.size(), symbol,
             from_date, to_date);
    return bars;
  };

  chisel::BacktestServiceImpl service(std::move(provider));

  grpc::ServerBuilder builder;
  builder.AddListeningPort(std::string{k_listen_addr},
                           grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    LOG_ERROR(logger, "Failed to start chisel_server on {}", k_listen_addr);
    return 1;
  }
  LOG_INFO(logger, "chisel_server listening on {}", k_listen_addr);
  std::println("chisel_server listening on {}", k_listen_addr);

  server->Wait();
  return 0;
}
