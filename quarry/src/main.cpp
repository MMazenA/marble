#include "aggregates.h"
#include "glaze/glaze.hpp"
#include "polygon.h"
#include "utils.h"
#include <print>

int main() {
  quarry::load_dotenv("./quarry/.env");
  quarry::Polygon polygon(std::getenv("POLYGON_API_KEY"));

  // constexpr auto aapl_daily_agg =
  // quarry::Endpoint::AggregatesRequest{"AAPL"};
  std::println("hi");

  /// @todo: need to save response here
  /// start and end days need to be passed through from front-end
  /// display stats about the selected period
  /// cache results into postgres so repeated selections don't consume tokens
}
