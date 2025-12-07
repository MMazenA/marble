#include "aggregates.h"
#include "base_endpoint.h"
#include "polygon.h"
#include "sql.h"
#include "utils.h"
#include <future>
#include <vector>

int main() {
  using Aggregates = quarry::ep::Aggregates;
  using AggBar = quarry::ep::AggBar;
  quarry::load_dotenv("./quarry/.env");
  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  auto aapl_daily_agg = Aggregates::with_ticker("AAPL")
                            .time_span(quarry::timespan_options::DAY)
                            .from_date("2024-01-01")
                            .to_date("2024-01-05");

  constexpr std::string table_name = "stg_aggregates_results";
  ;

  std::vector<std::future<void>> futures;

  for (const auto &aggregate_bar_batch :
       polygon.execute_with_pagination(aapl_daily_agg)) {
    const auto &bar_batch = *aggregate_bar_batch.results;
    futures.push_back(std::async(
        std::launch::async, quarry::Sql::bulk_insert<AggBar, AggBar::n_cols()>,
        bar_batch, table_name, AggBar::col_names()));
  }

  // program cannot end until these calls are resolved
  for (auto &fut : futures) {
    fut.get();
  }

  // for (const auto &aggregate_bar_batch :
  //      polygon.execute_with_pagination(aapl_daily_agg)) {
  //   for (const auto &aggregate_bar : (*aggregate_bar_batch.results)) {
  //     std::println("{}", aggregate_bar);
  //   }
  // }

  /// @todo:
  /// cache results into postgres so repeated selections don't consume tokens
}
