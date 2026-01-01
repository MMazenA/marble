#ifndef POLYGON_H
#define POLYGON_H
#include "base_endpoint.h"
#include "generator.h" // IWYU pragma: keep
#include "http_client.h"
#include "logging.h"
#include <glaze/glaze.hpp>
#include <memory>
#include <quill/LogMacros.h>
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
  explicit Polygon(std::string api_key);

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
      auto *logger = quarry::logging::get_logger();
      LOG_ERROR(logger, "JSON parse failed {}",
                glz::format_error(parsed_json, body_view));

      throw std::runtime_error("parse failed");
    }

    return *parsed_json;
  };

  template <quarry::endpoint_c E>
  std::generator<typename E::response_type>
  execute_with_pagination(const E &ep) {
    std::string url = m_authenticate_url(ep);

    while (!url.empty()) {
      http::response<http::string_body> result;
      if (ep.method() == quarry::method_type::GET) {
        result = m_http->get(url);
      } else {
        result = m_http->post(url);
      }

      std::string_view body_view = result.body();

      auto parsed_json = glz::read_json<typename E::response_type>(body_view);

      if (!parsed_json) {
        auto *logger = quarry::logging::get_logger();
        LOG_ERROR(logger, "JSON parse failed {}",
                  glz::format_error(parsed_json, body_view));
        throw std::runtime_error("parse failed");
      }

      co_yield *parsed_json;

      if (parsed_json->next_url) {
        url = parsed_json->next_url.value_or("");
        if (!url.empty()) {
          url = m_authenticate_url(url);
        }
      } else {
        url.clear();
      }
    }
  };

private:
  std::string m_api_key;

  template <quarry::endpoint_c E> std::string m_get_url(const E &ep) {
    const std::optional<std::string> validation = ep.validate();
    if (validation != std::nullopt) {
      throw std::invalid_argument(validation.value());
    }

    std::string full_url = ep.path() + ep.query();
    return full_url;
  }

  template <quarry::endpoint_c E> std::string m_authenticate_url(const E &ep) {
    std::string url = m_get_url(ep);
    return m_authenticate_url(url);
  };

  std::string m_authenticate_url(const std::string &url) {
    std::string api_key = "&apiKey=" + m_api_key;
    std::string full_url = url + api_key;
    return full_url;
  };

  std::unique_ptr<quarry::HttpClient> m_http;
};
} // namespace quarry

#endif
