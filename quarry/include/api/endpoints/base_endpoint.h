#ifndef BASE_ENDPOINT_H
#define BASE_ENDPOINT_H

#include <string>
#include <unordered_map>
#include <vector>

namespace quarry {

using headers = std::vector<std::pair<std::string, std::string>>;

enum sort_options { ASC, DSC };
enum timespan_options { SECOND, MINUTE, HOUR, DAY, WEEK, MONTH, QUARTER, YEAR };
inline std::string timespan_resolver(timespan_options timespan) {
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
class BaseEndpoint {
public:
  virtual std::string method() const = 0;
  virtual std::string target(std::string auth_path) const {
    return m_target_path() + "?" + m_target_args() + auth_path;
  };
  virtual std::string m_target_path() const = 0;
  virtual std::string m_target_args() const { return ""; }
  virtual std::string build_payload() const { return ""; }
  virtual headers build_headers() const { return {}; }
  virtual ~BaseEndpoint() = default;
}; // namespace quarry

} // namespace quarry

#endif
