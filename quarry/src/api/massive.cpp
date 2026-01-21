#include "api/massive.h"

quarry::Massive::Massive(std::string key)
    : m_api_key{std::move(key)},
      m_http{std::make_unique<quarry::HttpClient>("api.massive.io", 443)} {};
