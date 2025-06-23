#ifndef POLYGON_H
#define POLYGON_H

#include "endpoints/base_endpoint.h"
#include "http_client.h"

namespace quarry {

// Should contain higher level functions I.E
// get_minute_aggregates(?),
class Polygon {

public:
  Polygon(std::string api_key);

  std::string execute(const BaseEndpoint ep);

private:
  std::string m_api_key;
  std::string m_build_url(const BaseEndpoint ep);
};
} // namespace quarry

#endif
