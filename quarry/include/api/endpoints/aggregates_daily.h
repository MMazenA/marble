#ifndef AGGREGATES_DAILY_H
#define AGGREGATES_DAILY_H

#include "base_endpoint.h"
#include <iostream>
#include <sstream>
#include <string>

namespace quarry {

namespace Endpoint {
struct AggregatesDaily : public quarry::BaseEndpoint {

  // Path Params
  std::string m_ticker;
  int multiplier = 1;
  timespan_options m_timespan = DAY;
  std::string m_from_date;
  std::string m_to_date;

  // Query Params
  bool m_adjusted = true;
  sort_options m_sort = ASC;
  uint16_t m_limit = 120;

  /**
   * @brief Create a request for daily aggregate bars.
   *
   * @param ticker    Stock symbol (e.g., "AAPL").
   * @param from_date Start date (inclusive), format `YYYY-MM-DD`.
   * @param to_date   End date (inclusive), format `YYYY-MM-DD`.
   *
   * @see https://polygon.io/docs/rest/stocks/aggregates/custom-bars
   *
   * @todo: this should follow EXACTLY the above docs, and I should rename this
   * class to match it, theres no reason for me to be inventing my own stuff
   * here
   */
  AggregatesDaily(std::string ticker, std::string from_date,
                  std::string to_date)
      : m_ticker(std::move(ticker)) {
    constexpr int validDateSize = 10;
    if (from_date.size() != validDateSize || to_date.size() != validDateSize) {
      throw std::invalid_argument("Please configure dates properly");
    }
    if (from_date > to_date) {
      throw std::invalid_argument("Start date must be before end date");
    }

    m_from_date = from_date;
    m_to_date = to_date;
  }

  std::string method() const override { return "GET"; }

  std::string m_target_path() const override {
    std::ostringstream oss;
    oss << "/v2/aggs/ticker/" << m_ticker << "/range/" << multiplier << "/"
        << quarry::timespan_resolver(m_timespan) << "/" << m_from_date << "/"
        << m_to_date;
    return oss.str();
  }

  headers build_headers() const override {
    return {{"Accept", "application/json"}};
  }

  std::string m_target_args() const override {
    std::ostringstream oss;
    oss << "adjusted=" << (m_adjusted ? "true" : "false")
        << "&sort=" << (m_sort == sort_options::ASC ? "asc" : "desc")
        << "&limit=" << m_limit;
    return oss.str();
  }

  // Query Params
  AggregatesDaily &setSort(sort_options sort) {
    m_sort = sort;
    return *this;
  }

  AggregatesDaily &setAdjusted(bool adjusted) {
    m_adjusted = adjusted;
    return *this;
  }

  AggregatesDaily &setLimit(uint16_t limit) {
    m_limit = limit;
    return *this;
  }
};
} // namespace Endpoint
} // namespace quarry
#endif
