#ifndef AGGREGATES_DAILY_H
#define AGGREGATES_DAILY_H

#include "base_endpoint.h"
#include <sstream>

namespace quarry {

namespace Endpoint {
struct AggregatesDaily : public quarry::BaseEndpoint {

  std::string ticker;
  std::string from_date;
  std::string to_date;
  int multiplier = 1;
  timespan_options timespan = DAY;
  bool adjusted = true;
  sort_options sort = ASC;
  int limit = 120;

  AggregatesDaily(std::string ticker, std::string from_date,
                  std::string to_date)
      : ticker(std::move(ticker)), from_date(std::move(from_date)),
        to_date(std::move(to_date)) {}

  std::string method() const override { return "GET"; }

  std::string m_target_path() const override {
    std::ostringstream oss;
    oss << "/v2/aggs/ticker/" << ticker << "/range/" << multiplier << "/"
        << quarry::timespan_resolver(timespan) << "/" << from_date << "/"
        << to_date;
    return oss.str();
  }

  headers build_headers() const override {
    return {{"Accept", "application/json"}};
  }

  std::string m_target_args() const override {
    std::ostringstream oss;
    oss << "adjusted=" << (adjusted ? "true" : "false")
        << "&sort=" << (sort == sort_options::ASC ? "asc" : "desc")
        << "&limit=" << limit;
    return oss.str();
  }
};
} // namespace Endpoint
} // namespace quarry
#endif
