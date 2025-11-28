#include "api/http_client.h"
#include "api/http_request_builder.h"
#include "api/transport.h"
#include "dns_cache.h"
#include "http_types.h"
#include "ssl_context_provider.h"
#include <format>
#include <iostream>
#include <stdexcept>

namespace quarry {
// resolve dns endpoint
// generate a stream to the endpoint
// write request to the stream
// read response from stream

HttpClient::HttpClient(std::string host, port_type port)
    : m_host(std::move(host)), m_port(port),
      m_ssl_ioc(quarry::SslContextProvider::make_client_ctx()) {}

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
  if (params.port == 443) {
    return m_https_client(params);
  }

  try {
    auto ioc = net::io_context();

    DnsCacheContext context{
        .host = params.host,
        .ioc = ioc,
        .port = params.port,
        .is_tls = false,
    };
    const tcp_resolver_results &endpoints =
        quarry::DnsCache::global_cache()->get(context);

    Transport transport(ioc);
    transport.connect(endpoints);

    auto req = HttpRequestBuilder{}
                   .verb(params.verb)
                   .target(params.target)
                   .version(11)
                   .host(params.host)
                   .user_agent(BOOST_BEAST_VERSION_STRING)
                   .headers(params.headers)
                   .build();

    transport.write(req);
    transport.read(params.http_response);

    return params.http_response.result_int();
  } catch (std::exception const &e) {
    std::cerr << "HTTP error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}

u_int HttpClient::m_https_client(const HttpRequestParams &params) {

  try {
    net::io_context ioc;
    DnsCacheContext context{
        .host = params.host,
        .ioc = ioc,
        .port = params.port,
        .is_tls = true,
    };
    const tcp_resolver_results endpoints =
        quarry::DnsCache::global_cache()->get(context);
    Transport transport(std::string{context.host}, context.ioc, m_ssl_ioc);
    transport.connect(endpoints);

    auto req = HttpRequestBuilder{}
                   .verb(params.verb)
                   .target(params.target)
                   .version(11)
                   .host(params.host)
                   .user_agent(BOOST_BEAST_VERSION_STRING)
                   .headers(params.headers)
                   .build();

    transport.write(req);
    transport.read(params.http_response);

  } catch (std::exception const &e) {
    std::cerr << "HTTPS error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return params.http_response.result_int();
}

} // namespace quarry
