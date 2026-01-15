#ifndef AGGREGATES_DAILY_H
#define AGGREGATES_DAILY_H

#include "base_endpoint.h"
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace quarry::ep {

struct AggBar {
  double o;
  double c;
  double h;
  double l;
  std::int64_t n;
  std::int64_t t;
  bool otc;
  double v;
  double vw;

  [[nodiscard]] auto to_tuple() const {
    return std::make_tuple(o, c, h, l, n, otc, t, v, vw);
  }

  constexpr static std::array<std::string, 9> col_names() {
    return {"o", "c", "h", "l", "n", "otc", "t", "v", "vw"};
  }

  constexpr static size_t n_cols() { return 9; }
};

struct AggregatesR {
  std::string ticker;
  bool adjusted;
  int queryCount;
  std::string request_id;
  int resultsCount;
  int count;
  std::string status;
  std::optional<std::vector<AggBar>> results;
  std::optional<std::string> next_url;
};

/**
 * @brief Create a request for aggregate bars.
 *
 * @attention Use `with_ticker` to begin builder
 *
 * @see https://polygon.io/docs/rest/stocks/aggregates/custom-bars
 */
class Aggregates {
private:
  Aggregates() = default;

public:
  std::string m_ticker;
  std::string m_from_date;
  std::string m_to_date;
  unsigned int m_multiplier = 1;
  timespan_options m_timespan = timespan_options::DAY;
  bool m_adjusted = true;
  sort_options m_sort = sort_options::ASC;
  uint16_t m_limit = 120;

  using response_type = AggregatesR;

  [[nodiscard]] static Aggregates with_ticker(std::string_view ticker) {
    if (ticker.empty()) {
      throw std::invalid_argument("ticker can not be empty");
    }
    Aggregates ep;
    ep.m_ticker.assign(ticker);
    return ep;
  }

  [[nodiscard]] static constexpr quarry::method_type method() noexcept {
    return quarry::method_type::GET;
  }

  [[nodiscard]] std::string path() const {
    if (m_ticker.empty()) {
      throw std::logic_error("ticker is empty");
    }
    if (!m_from_date.empty() && !quarry::is_iso_date(m_from_date)) {
      throw std::logic_error("from_date not ISO YYYY-MM-DD");
    }
    if (!m_to_date.empty() && !quarry::is_iso_date(m_to_date)) {
      throw std::logic_error("to_date not ISO YYYY-MM-DD");
    }

    std::string path;
    path.reserve(64 + m_ticker.size() + m_from_date.size() + m_to_date.size());
    path += "/v2/aggs/ticker/";
    path += m_ticker;
    path += "/range/";
    path += std::to_string(m_multiplier == 0 ? 1U : m_multiplier);
    path += "/";
    path += quarry::timespan_resolver(m_timespan);
    path += "/";
    path += m_from_date;
    path += "/";
    path += m_to_date;
    return path;
  }

  [[nodiscard]] static quarry::headers headers() {
    return {{"Accept", "application/json"}};
  }

  [[nodiscard]] std::string query() const {
    std::string query;
    query.reserve(48);
    query += "?adjusted=";
    query += (m_adjusted ? "true" : "false");
    query += "&sort=";
    query += (m_sort == sort_options::ASC ? "asc" : "desc");
    query += "&limit=";
    query += std::to_string(m_limit == 0 ? 1U : m_limit);
    return query;
  }

  [[nodiscard]] std::optional<std::string> validate() const noexcept {
    if (m_ticker.empty()) {
      return "ticker empty";
    }
    if (m_multiplier == 0) {
      return "multiplier must be > 0";
    }
    if (m_limit == 0) {
      return "limit must be > 0";
    }
    if (!m_from_date.empty() && !quarry::is_iso_date(m_from_date)) {
      return "from_date not ISO YYYY-MM-DD";
    }
    if (!m_to_date.empty() && !quarry::is_iso_date(m_to_date)) {
      return "to_date not ISO YYYY-MM-DD";
    }
    return std::nullopt;
  }

  // builders
  [[nodiscard]] Aggregates &sort(sort_options s) noexcept {
    m_sort = s;
    return *this;
  }
  [[nodiscard]] Aggregates &adjusted(bool a) noexcept {
    m_adjusted = a;
    return *this;
  }
  [[nodiscard]] Aggregates &limit(uint16_t lim) noexcept {
    m_limit = (lim != 0U) ? lim : 1;
    return *this;
  }
  [[nodiscard]] Aggregates &from_date(std::string d) {
    if (!d.empty() && !quarry::is_iso_date(d)) {
      throw std::invalid_argument("Bad ISO date for fromDate: " + d);
    }
    m_from_date = std::move(d);
    return *this;
  }
  [[nodiscard]] Aggregates &to_date(std::string d) {
    if (!d.empty() && !quarry::is_iso_date(d)) {
      throw std::invalid_argument("Bad ISO date for toDate: " + d);
    }
    m_to_date = std::move(d);
    return *this;
  }
  [[nodiscard]] Aggregates &time_span(timespan_options t) noexcept {
    m_timespan = t;
    return *this;
  }
  [[nodiscard]] Aggregates &multiplier(unsigned int m) noexcept {
    m_multiplier = (m != 0U) ? m : 1;
    return *this;
  }

  /// @todo add r-value && operators so builder pattern can be expanded to
  /// invalidate built objects
};

static_assert(bulk_uploadable_c<AggBar>);

} // namespace quarry::ep

template <>
struct std::formatter<quarry::ep::AggBar> : std::formatter<std::string> {
  auto format(const quarry::ep::AggBar &ab, auto &ctx) const {

    std::string formatted_ab =
        std::format("[Aggregate Bar {{"
                    "open:{:<6} close:{:<6} "
                    "high:{:<6} low:{:<6} transactions_c:{:<6} "
                    "volume:{:<6} weighted_volume:{:<8} "
                    "is_otc:{:<5} timestamp:{}}}]",
                    ab.o, ab.c, ab.h, ab.l, ab.n, ab.v, ab.vw, ab.otc, ab.t);

    // return std::formatter<std::string>::format
    return std::formatter<std::string>::format(formatted_ab, ctx);
  }
};

#endif
