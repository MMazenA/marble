#include "api/http_client.h"
#include "dns_cache.h"
#include "http_types.h"
#include "stream_guard.h"
#include <format>
#include <iostream>
#include <stdexcept>

namespace quarry {

HttpClient::HttpClient(std::string host, port_type port)
    : m_host(std::move(host)), m_port(port), m_ssl_ioc(m_make_client_ctx()) {}

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
    int version = 11;

    DnsCacheContext context{
        .host = params.host,
        .ioc = ioc,
        .port = params.port,
        .is_tls = false,
    };
    StreamGuard stream_guard(context.ioc);
    const tcp_resolver_results &endpoints =
        quarry::DnsCache::global_cache()->get(context);
    stream_guard.connect(endpoints);

    http::request<http::string_body> req{params.verb, params.target, version};

    req.set(http::field::host, params.host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    for (const auto &[k, v] : params.headers) {
      req.set(k, v);
    }

    http::write(stream_guard.get<tcp_stream>(), req); // check no copy

    beast::flat_buffer buffer;

    http::read(stream_guard.get<tcp_stream>(), buffer,
               params.http_response); // check no copy

    return params.http_response.result_int();
  } catch (std::exception const &e) {
    std::cerr << "HTTP error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}

u_int HttpClient::m_https_client(const HttpRequestParams &params) {

  // io context - contains context to execute the event loop and all system
  try {

    // calls needed
    // https://app.studyraid.com/en/read/12426/401278/understanding-io-context-and-its-lifecycle
    // https://stackoverflow.com/questions/60997939/what-exacty-is-io-context
    // ssl handshake context
    net::io_context ioc;
    DnsCacheContext context{
        .host = params.host,
        .ioc = ioc,
        .port = params.port,
        .is_tls = true,
    };

    StreamGuard stream_guard(std::string{context.host}, context.ioc, m_ssl_ioc);
    const tcp_resolver_results endpoints =
        quarry::DnsCache::global_cache()->get(context);
    stream_guard.connect(endpoints);

    http::request<http::string_body> req{params.verb, params.target, 11};
    req.set(http::field::host, params.host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    for (const auto &[k, v] : params.headers) {
      req.set(k, v);
    }

    http::write(stream_guard.get<tls_stream>(), req);

    beast::flat_buffer buffer;
    http::read(stream_guard.get<tls_stream>(), buffer, params.http_response);

  } catch (std::exception const &e) {
    std::cerr << "HTTPS error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return params.http_response.result_int();
}

ssl::context HttpClient::m_make_client_ctx() {
  ssl::context ctx{ssl::context::tls_client};
  ctx.set_default_verify_paths();
  ctx.set_verify_mode(ssl::verify_peer);

  SSL_CTX_set_session_cache_mode(ctx.native_handle(), SSL_SESS_CACHE_CLIENT);
  SSL_CTX_sess_set_cache_size(ctx.native_handle(), 128);

  return ctx;
}

} // namespace quarry
