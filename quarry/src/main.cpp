#include "aggregates.h"
#include "polygon.h"
#include "utils.h"
#include <print>

int main() {
  using aggregates = quarry::ep::Aggregates;
  quarry::load_dotenv("./quarry/.env");
  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  auto aapl_daily_agg = aggregates::with_ticker("AAPL")
                            .from_date("2024-01-01")
                            .to_date("2025-01-01");

  for (const auto &aggregate_bar_batch :
       polygon.execute_with_pagination(aapl_daily_agg)) {
    for (const auto &aggregate_bar : (*aggregate_bar_batch.results)) {
      std::println("{}", aggregate_bar);
    }
  }

  /// @todo:
  /// cache results into postgres so repeated selections don't consume tokens
}
