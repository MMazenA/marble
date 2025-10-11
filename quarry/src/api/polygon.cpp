#include "api/polygon.h"

quarry::Polygon::Polygon(std::string key)
    : m_api_key{key},
      m_http{std::make_unique<quarry::httpClient>("api.polygon.io", "443")} {};
