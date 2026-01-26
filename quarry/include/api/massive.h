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

/**
 * Rule of zero - movable via unique_ptr, non-copyable.
 */
class Massive {

public:
  /**
   * @brief Massive API Client
   *
   * @param api_key Massive API Key
   *
   */
  explicit Massive(std::string api_key);

  Massive(Massive &&) noexcept = default;
  Massive &operator=(Massive &&) noexcept = default;

  Massive(const Massive &other) = delete;
  Massive &operator=(const Massive &other) = delete;

  ~Massive() noexcept = default;

  template <quarry::endpoint_c E>
  auto execute(const E &ep) -> E::response_type {
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
  auto execute_with_pagination(const E &ep)
      -> std::generator<typename E::response_type> {
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

  template <quarry::endpoint_c E> auto m_get_url(const E &ep) -> std::string {
    const std::expected<bool, std::string_view> validation = ep.validate();
    if (validation.has_value()) {
      std::string full_url = ep.path() + ep.query();
      return full_url;
    }

    throw std::invalid_argument(std::string{validation.error()});
  }

  template <quarry::endpoint_c E>
  auto m_authenticate_url(const E &ep) -> std::string {
    std::string url = m_get_url(ep);
    return m_authenticate_url(url);
  };

  auto m_authenticate_url(const std::string &url) -> std::string {
    constexpr std::string_view api_key_prefix = "&apiKey=";

    std::string out;
    out.reserve(url.size() + m_api_key.size() + api_key_prefix.size());

    out += url;
    out += api_key_prefix;
    out += m_api_key;

    return out;
  }

  std::unique_ptr<quarry::HttpClient> m_http;
};
} // namespace quarry

#endif
