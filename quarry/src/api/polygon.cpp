#include "api/polygon.h"

quarry::Polygon::Polygon(std::string key)
    : m_api_key{key},
      m_http{std::make_unique<quarry::httpClient>("api.polygon.io", "443")} {};

std::string quarry::Polygon::m_authenticate_url(const BaseEndpoint &ep) {
  std::string api_key = "&apiKey=" + m_api_key;
  return ep.target(api_key);
};
std::string quarry::Polygon::execute(const BaseEndpoint &ep) {
  std::string url = m_authenticate_url(ep);
  http::response<http::dynamic_body> result;
  if (ep.method() == "GET") {
    result = m_http->get(url);
  } else {
    result = m_http->post(url);
  }

  std::cout << result << std::endl;
  /// @todo return the correct thing here
  return "hi";
};
