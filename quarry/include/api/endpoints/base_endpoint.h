#ifndef BASE_ENDPOINT_H
#define BASE_ENDPOINT_H

#include <concepts>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace quarry {

using headers = std::vector<std::pair<std::string, std::string>>;

enum class sort_options { ASC, DSC };
enum class method_type { GET, POST };
enum class timespan_options : uint8_t {
  SECOND,
  MINUTE,
  HOUR,
  DAY,
  WEEK,
  MONTH,
  QUARTER,
  YEAR
};

struct polygonRequest {
  method_type method;
  std::string path;
  std::string query;
  headers hdrs;
};

constexpr uint8_t ticker_size = 6;

constexpr std::string_view
timespan_resolver(timespan_options timespan) noexcept {
  switch (timespan) {
  case timespan_options::SECOND:
    return "second";
  case timespan_options::MINUTE:
    return "minute";
  case timespan_options::HOUR:
    return "hour";
  case timespan_options::DAY:
    return "day";
  case timespan_options::WEEK:
    return "week";
  case timespan_options::MONTH:
    return "month";
  case timespan_options::QUARTER:
    return "quarter";
  case timespan_options::YEAR:
    return "year";
  }
  return "day";
}

[[nodiscard]] inline bool is_iso_date(std::string_view s) noexcept {
  if (s[4] != '-' || s[7] != '-') {
    return false;
  }
  auto is_digit = [](char c) constexpr noexcept {
    return c >= '0' && c <= '9';
  };
  return is_digit(s[0]) && is_digit(s[1]) && is_digit(s[2]) && is_digit(s[3]) &&
         is_digit(s[5]) && is_digit(s[6]) && is_digit(s[8]) && is_digit(s[9]);
}

template <class T>
concept endpoint_c = requires(const T &ep) {
  { ep.method() } -> std::same_as<method_type>;
  { ep.path() } -> std::convertible_to<std::string_view>;
  { ep.query() } -> std::convertible_to<std::string_view>;
  { ep.headers() } -> std::same_as<headers>;
  { ep.validate() } -> std::same_as<std::optional<std::string>>;
  typename T::response_type;
};

struct aggBar {
  double o;
  double c;
  double h;
  double l;
  std::int64_t n;
  bool otc;
  std::int64_t t;
  double v;
  double vw;
};

} // namespace quarry

template <>
struct std::formatter<quarry::aggBar> : std::formatter<std::string> {
  auto format(const quarry::aggBar &ab, auto &ctx) const {

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
