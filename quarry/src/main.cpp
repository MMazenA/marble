#include "aggregates.h"
#include "base_endpoint.h"
#include "polygon.h"
#include "sql.h"
#include "utils.h"
#include <future>
#include <optional>
#include <string_view>
#include <vector>

int main() {
  using Aggregates = quarry::ep::Aggregates;
  using AggBar = quarry::ep::AggBar;
  quarry::load_dotenv("./quarry/.env");
  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  auto aapl_daily_agg = Aggregates::with_ticker("AAPL")
                            .time_span(quarry::timespan_options::DAY)
                            .from_date("2024-01-01")
                            .to_date("2025-01-05");

  const std::string table_name = "stg_aggregates_results";

  std::vector<std::future<void>> futures;
  std::optional<std::string> last_request_id;
  std::string last_ticker;
  bool staged_rows = false;

  for (const auto &aggregate_bar_batch :
       polygon.execute_with_pagination(aapl_daily_agg)) {
    if (!aggregate_bar_batch.results.has_value() ||
        aggregate_bar_batch.results->empty()) {
      continue;
    }

    const auto &bar_batch = *aggregate_bar_batch.results;
    last_ticker = aggregate_bar_batch.ticker;
    last_request_id = aggregate_bar_batch.request_id;
    staged_rows = true;

    futures.push_back(std::async(
        std::launch::async, quarry::Sql::bulk_insert<AggBar, AggBar::n_cols()>,
        bar_batch, table_name, AggBar::col_names()));
  }

  for (auto &fut : futures) {
    fut.get();
  }

  if (staged_rows && !last_ticker.empty()) {
    std::optional<std::string_view> req_id_view = std::nullopt;
    if (last_request_id.has_value()) {
      req_id_view = std::string_view{*last_request_id};
    }
    quarry::Sql::normalize_staged_aggregates(last_ticker, req_id_view,
                                             std::string_view{last_ticker});
  }
}
