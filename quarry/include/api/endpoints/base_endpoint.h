#ifndef BASE_ENDPOINT_H
#define BASE_ENDPOINT_H

#include <array>
#include <concepts>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace quarry {

using headers = std::vector<std::pair<std::string, std::string>>;

enum class sort_options : uint8_t { ASC, DSC };
enum class method_type : uint8_t { GET, POST };
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

constexpr uint8_t ticker_size = 6;

[[nodiscard]] constexpr std::string_view
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
  std::unreachable();
}

[[nodiscard]] constexpr timespan_options
timespan_resolver(uint8_t value) noexcept {
  // NOLINTNEXTLINE(bugprone-switch-missing-default-case)
  switch (value) {
  case 0:
    return timespan_options::SECOND;
  case 1:
    return timespan_options::MINUTE;
  case 2:
    return timespan_options::HOUR;
  case 3:
    return timespan_options::DAY;
  case 4:
    return timespan_options::WEEK;
  case 5:
    return timespan_options::MONTH;
  case 6:
    return timespan_options::QUARTER;
  case 7:
    return timespan_options::YEAR;
  default:
    return timespan_options::MONTH;
  }
  std::unreachable();
}

[[nodiscard]] constexpr bool is_iso_date(const std::string_view s) noexcept {
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
  { ep.validate() } -> std::same_as<std::expected<bool, std::string_view>>;
  typename T::response_type;
};

template <class T>
concept bulk_uploadable_c = requires(const T &t) {
  t.to_tuple();
  { t.n_cols() } -> std::same_as<std::size_t>;
  { T::col_names() } -> std::same_as<std::array<std::string, T::n_cols()>>;
};

} // namespace quarry

#endif
