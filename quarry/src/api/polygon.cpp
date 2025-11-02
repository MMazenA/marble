#include "api/polygon.h"

quarry::Polygon::Polygon(std::string key)
    : m_api_key{std::move(key)},
      m_http{std::make_unique<quarry::HttpClient>("api.polygon.io", "443")} {};
