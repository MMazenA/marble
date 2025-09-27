#ifndef POLYGON_H
#define POLYGON_H

#include "endpoints/base_endpoint.h"
#include "http_client.h"
#include <memory>
#include <string>

namespace quarry {

// Should contain higher level functions I.E
// get_minute_aggregates(?),
class Polygon {

public:
  /**
   * @brief Polygon API Client
   *
   * @param api_key Polygon API Key
   *
   */
  Polygon(std::string api_key);

  std::string execute(const BaseEndpoint &ep);

private:
  std::string m_api_key;
  std::string m_authenticate_url(const BaseEndpoint &ep);
  std::unique_ptr<quarry::httpClient> m_http;
};
} // namespace quarry

#endif
