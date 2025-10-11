#include "aggregates.h"
#include "glaze/glaze.hpp"
#include "polygon.h"
#include "utils.h"
#include <print>

int main() {
  using aggregates = quarry::ep::Aggregates;
  quarry::load_dotenv("./quarry/.env");
  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  auto aapl_daily_agg = aggregates::withTicker("AAPL")
                            .fromDate("2024-01-09")
                            .toDate("2024-01-11");
  aggregates::response_type res = polygon.execute(aapl_daily_agg);

  for (const auto &aggregate_bar : (*res.results)) {
    std::println("{}", aggregate_bar);
  }

  /// @todo:
  /// cache results into postgres so repeated selections don't consume tokens
}
