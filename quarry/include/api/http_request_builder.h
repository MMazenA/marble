#ifndef QUARRY_API_HTTP_REQUEST_BUILDER_H
#define QUARRY_API_HTTP_REQUEST_BUILDER_H
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>
#include <unordered_map>

namespace quarry {

namespace http = boost::beast::http;
class HttpRequestBuilder {

public:
  [[nodiscard]] HttpRequestBuilder &verb(http::verb verb) noexcept;
  [[nodiscard]] HttpRequestBuilder &target(std::string_view target);
  [[nodiscard]] HttpRequestBuilder &version(int version) noexcept;
  [[nodiscard]] HttpRequestBuilder &host(std::string_view host_name);
  [[nodiscard]] HttpRequestBuilder &user_agent(std::string_view user_agent);
  [[nodiscard]] HttpRequestBuilder &keep_alive(bool keep_alive) noexcept;
  [[nodiscard]] HttpRequestBuilder &
  headers(const std::unordered_map<std::string, std::string> &headers);
  [[nodiscard]] HttpRequestBuilder &body(std::string_view body);

  [[nodiscard]] http::request<http::string_body> build() const;

private:
  http::verb m_verb{http::verb::get};
  std::string m_target{"/"};
  int m_version{11};
  std::string m_host_name;
  std::string m_user_agent{BOOST_BEAST_VERSION_STRING};
  std::unordered_map<std::string, std::string> m_headers;
  std::string m_body;
  bool m_keep_alive{false};
};

} // namespace quarry

#endif
