#include "api/http_client.h"
#include "api/http_request_builder.h"
#include "api/transport.h"
#include "dns_cache.h"
#include "http_types.h"
#include "logging.h"
#include "transport_pool.h"
#include <format>
#include <optional>
#include <quill/LogMacros.h>
#include <stdexcept>
#include <utility>

namespace quarry {
// NOLINTNEXTLINE
constexpr int HTTP_TLS_POOL_SIZE = 5;

HttpClient::HttpClient(std::string host, port_type port, bool is_tls,
                       const std::function<ssl::context()> &ctx_provider)
    : m_host(std::move(host)), m_port(port), m_ssl_ioc(ctx_provider()),
      m_is_tls(is_tls || port == 443) {

  DnsCacheContext context{
      .host = m_host,
      .ioc = m_ioc,
      .port = m_port,
      .is_tls = m_is_tls,
  };
  const tcp_resolver_results endpoints =
      quarry::DnsCache::global_cache().get(context);

  if (m_is_tls) {
    m_transport_pool_tls.emplace(HTTP_TLS_POOL_SIZE, m_host, m_ioc, m_ssl_ioc,
                                 endpoints);
  }
}

/// @brief `get` request to v2/endpoint/example and headers
/// @param endpoint
/// @param headers
/// @return
http::response<http::string_body>
HttpClient::get(const std::string_view endpoint,
                const std::unordered_map<std::string, std::string> &headers) {
  http::response<http::string_body> response;

  HttpRequestParams params{
      .host = m_host,
      .port = m_port,
      .target = endpoint,
      .verb = http::verb::get,
      .headers = headers,
      .http_response = response,
  };

  u_int response_code = m_client(params);

  if (response_code != 200) {
    // @todo refactor this class and callers to use std::expected, i dont want
    // to throw runtime errors and kill threads
    auto *logger = quarry::logging::get_logger();
    LOG_ERROR(logger, "HTTP {} for {}: {}", response_code, endpoint,
              response.body());

    throw std::runtime_error(std::format("HTTP Error code: {}", response_code));
  }

  return response;
};

http::response<http::string_body>
HttpClient::post(const std::string_view endpoint, const std::string_view body,
                 const std::unordered_map<std::string, std::string> &headers) {
  http::response<http::string_body> response;

  HttpRequestParams params{
      .host = m_host,
      .port = m_port,
      .target = endpoint,
      .verb = http::verb::post,
      .headers = headers,
      .http_response = response,
  };

  u_int response_code = m_client(params);

  if (response_code != 200) {
    throw std::runtime_error(std::format("HTTP Error code: {}", response_code));
  }

  return response;
};

/// @brief Primary client for connecting to endpoints
/// @param params HttpRequestParams containing host, port, target, verb,
/// headers, and response
/// @return https response code
u_int HttpClient::m_client(const HttpRequestParams &params) {
  if (m_is_tls) {
    return m_https_client(params);
  }

  auto ioc = net::io_context();

  DnsCacheContext context{
      .host = params.host,
      .ioc = ioc,
      .port = params.port,
      .is_tls = false,
  };
  const tcp_resolver_results &endpoints =
      quarry::DnsCache::global_cache().get(context);

  Transport transport(ioc);
  transport.connect(endpoints);

  auto req = HttpRequestBuilder{}
                 .verb(params.verb)
                 .target(params.target)
                 .version(11)
                 .host(params.host)
                 .user_agent(BOOST_BEAST_VERSION_STRING)
                 .keep_alive(true)
                 .headers(params.headers)
                 .build();

  transport.write_and_read(req, params.http_response);

  return params.http_response.result_int();
}

u_int HttpClient::m_https_client(const HttpRequestParams &params) {
  auto req = HttpRequestBuilder{}
                 .verb(params.verb)
                 .target(params.target)
                 .version(11)
                 .host(params.host)
                 .user_agent(BOOST_BEAST_VERSION_STRING)
                 .keep_alive(true)
                 .headers(params.headers)
                 .build();

  m_transport_pool_tls->send_and_read(req, params.http_response);

  return params.http_response.result_int();
}

} // namespace quarry
