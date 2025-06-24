#include "api/polygon.h"

quarry::Polygon::Polygon(std::string key)
    : m_api_key{key},
      m_http{std::make_unique<quarry::httpClient>("api.polygon.io", "80")} {};

std::string quarry::Polygon::m_authenticate_url(const BaseEndpoint &ep) {
  return ep.path() + "/apiKey=" + m_api_key;
};
std::string quarry::Polygon::execute(const BaseEndpoint &ep) {
  std::string url = m_authenticate_url(ep);
  std::cout << url << std::endl;
  auto result = m_http->get(url);

  std::cout << result << std::endl;
  return "hi";
};
