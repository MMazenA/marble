#ifndef BASE_ENDPOINT_H
#define BASE_ENDPOINT_H

#include <string>
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
}

[[nodiscard]] inline bool is_iso_date(std::string_view s) noexcept {
  return s.size() == 10 && s[4] == '-' && s[7] == '-' && (s[0] | 32) >= '0' &&
         (s[0] | 32) <= '9';
}

template <class T>
concept endpoint_c = requires(const T &ep) {
  { ep.method() } -> std::convertible_to<std::string_view>;
  { ep.path() } -> std::convertible_to<std::string_view>;
  { ep.query() } -> std::convertible_to<std::string_view>;
  { ep.headers() } -> std::same_as<headers>;
  { ep.validate() } -> std::same_as<std::optional<std::string>>;
};

inline polygonRequest build_request(const endpoint_c auto &ep) {
  int x;

  return polygonRequest{std::string(ep.method()), std::string(ep.path()),
                        std::string(ep.query()), ep.headers()};
}

} // namespace quarry

#endif
