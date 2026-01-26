// engine code for main

#include <cstdlib>
#include <string>

#include "api/endpoints/aggregates.h"
#include "api/massive.h"
#include "logging.h"
#include "utils.h"

void example() {
  quarry::logging::Config log_config{};
  log_config.logger_name = "chisel";
  log_config.log_path = "logs/chisel.log";
  auto *logger = quarry::logging::init(log_config);
  quarry::load_dotenv();
  const char *api_key = std::getenv("MASSIVE_API_KEY");

  if ((api_key == nullptr) || std::string_view(api_key).empty()) {
    LOG_ERROR(logger, "MASSIVE_API_KEY is not set");
    return;
  }

  quarry::Massive massive(api_key);

  auto ep = quarry::ep::Aggregates::with_ticker("AAPL")
                .time_span(quarry::timespan_options::DAY)
                .from_date("2025-01-01")
                .to_date("2025-01-05");

  try {
    const auto response = massive.execute(ep);
    const auto count = response.results ? response.results->size() : 0;
    LOG_INFO(logger, "Fetched {} aggregate bars", count);
  } catch (const std::exception &ex) {
    LOG_ERROR(logger, "Massive request failed: {}", ex.what());
    return;
  }
}

int main() {
  example();
  return 0;
}
