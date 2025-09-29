#ifndef AGGREGATES_DAILY_H
#define AGGREGATES_DAILY_H

#include "base_endpoint.h"
#include <sstream> // ostringstream
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility> // std::move

namespace quarry {
namespace Endpoint {

/**
 * @brief Create a request for aggregate bars.
 *
 * @attention Use `withTicker` to begin builder
 *
 * @see https://polygon.io/docs/rest/stocks/aggregates/custom-bars
 */
struct AggregatesRequest {
private:
  AggregatesRequest() = default;

  std::string m_ticker;
  std::string m_from_date;
  std::string m_to_date;
  unsigned int m_multiplier = 1;
  timespan_options m_timespan = timespan_options::DAY;
  bool m_adjusted = true;
  sort_options m_sort = sort_options::ASC;
  uint16_t m_limit = 120;

public:
  [[nodiscard]] static AggregatesRequest withTicker(std::string_view ticker) {
    if (ticker.empty())
      throw std::invalid_argument("ticker can not be empty");
    AggregatesRequest ep;
    ep.m_ticker.assign(ticker);
    return ep;
  }

  // concept requirements
  [[nodiscard]] static constexpr std::string_view method() noexcept {
    return "GET";
  }

  [[nodiscard]] std::string path() const {
    if (m_ticker.empty())
      throw std::logic_error("ticker is empty");
    if (!m_from_date.empty() && !quarry::is_iso_date(m_from_date))
      throw std::logic_error("from_date not ISO YYYY-MM-DD");
    if (!m_to_date.empty() && !quarry::is_iso_date(m_to_date))
      throw std::logic_error("to_date not ISO YYYY-MM-DD");

    std::string path;
    path.reserve(64 + m_ticker.size() + m_from_date.size() + m_to_date.size());
    path += "/v2/aggs/ticker/";
    path += m_ticker;
    path += "/range/";
    path += std::to_string(m_multiplier == 0 ? 1u : m_multiplier);
    path += "/";
    path += quarry::timespan_resolver(m_timespan);
    path += "/";
    path += m_from_date;
    path += "/";
    path += m_to_date;
    return path;
  }

  [[nodiscard]] quarry::headers headers() const {
    return {{"Accept", "application/json"}};
  }

  [[nodiscard]] std::string query() const {
    std::string query;
    query.reserve(48);
    query += "adjusted=";
    query += (m_adjusted ? "true" : "false");
    query += "&sort=";
    query += (m_sort == sort_options::ASC ? "asc" : "desc");
    query += "&limit=";
    query += std::to_string(m_limit == 0 ? 1u : m_limit);
    return query;
  }

  [[nodiscard]] std::optional<std::string> validate() const noexcept {
    if (m_ticker.empty())
      return "ticker empty";
    if (m_multiplier == 0)
      return "multiplier must be > 0";
    if (m_limit == 0)
      return "limit must be > 0";
    if (!m_from_date.empty() && !quarry::is_iso_date(m_from_date))
      return "from_date not ISO YYYY-MM-DD";
    if (!m_to_date.empty() && !quarry::is_iso_date(m_to_date))
      return "to_date not ISO YYYY-MM-DD";
    return std::nullopt;
  }

  // builders
  [[nodiscard]] AggregatesRequest &sort(sort_options s) noexcept {
    m_sort = s;
    return *this;
  }
  [[nodiscard]] AggregatesRequest &adjusted(bool a) noexcept {
    m_adjusted = a;
    return *this;
  }
  [[nodiscard]] AggregatesRequest &limit(uint16_t lim) noexcept {
    m_limit = lim ? lim : 1;
    return *this;
  }
  [[nodiscard]] AggregatesRequest &fromDate(std::string d) {
    if (!d.empty() && !quarry::is_iso_date(d))
      throw std::invalid_argument("Bad ISO date for fromDate");
    m_from_date = std::move(d);
    return *this;
  }
  [[nodiscard]] AggregatesRequest &toDate(std::string d) {
    if (!d.empty() && !quarry::is_iso_date(d))
      throw std::invalid_argument("Bad ISO date for toDate");
    m_to_date = std::move(d);
    return *this;
  }
  [[nodiscard]] AggregatesRequest &timeSpan(timespan_options t) noexcept {
    m_timespan = t;
    return *this;
  }
  [[nodiscard]] AggregatesRequest &multiplier(unsigned int m) noexcept {
    m_multiplier = m ? m : 1;
    return *this;
  }

  /// @todo add r-value && operators so builder pattern can be expanded to
  /// invalidate built objects
};

struct AggregatesResponse {};

} // namespace Endpoint
} // namespace quarry
#endif
