#ifndef POLYGON_H
#define POLYGON_H

#include "base_endpoint.h"
#include "http_client.h"
#include <glaze/glaze.hpp>
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

  template <quarry::endpoint_c E> E::response_type execute(const E &ep) {
    std::string url = m_authenticate_url(ep);

    http::response<http::string_body> result;
    if (ep.method() == quarry::method_type::GET) {
      result = m_http->get(url);
    } else {
      result = m_http->post(url);
    }

    std::string_view body_view = result.body();

    auto parsed_json = glz::read_json<typename E::response_type>(body_view);

    if (!parsed_json) {
      const auto &e = parsed_json.error();
      std::cerr << "JSON parse failed "
                << glz::format_error(parsed_json, body_view) << std::endl;

      throw std::runtime_error("parse failed");
    }

    return std::move(*parsed_json);
  };

private:
  std::string m_api_key;

  template <quarry::endpoint_c E> std::string m_authenticate_url(const E &ep) {
    std::string api_key = "&apiKey=" + m_api_key;

    const std::optional<std::string> validation = ep.validate();
    if (validation != std::nullopt) {
      throw std::invalid_argument(validation.value());
    }

    std::string full_url = ep.path() + ep.query() + api_key;
    return full_url;
  };

  std::unique_ptr<quarry::httpClient> m_http;
};
} // namespace quarry

#endif
