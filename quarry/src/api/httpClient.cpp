#include "api/http_client.h"
#include "http_types.h"
#include "stream_guard.h"
#include <format>
#include <iostream>
#include <stdexcept>

namespace quarry {

HttpClient::HttpClient(std::string host, port_type port)
    : m_host(std::move(host)), m_port(port), m_ssl_ioc(m_make_client_ctx()) {}

ssl::context HttpClient::m_make_client_ctx() {
  ssl::context ctx{ssl::context::tls_client};
  ctx.set_default_verify_paths();
  ctx.set_verify_mode(ssl::verify_peer);

  SSL_CTX_set_session_cache_mode(ctx.native_handle(), SSL_SESS_CACHE_CLIENT);
  SSL_CTX_sess_set_cache_size(ctx.native_handle(), 128);

  return ctx;
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
  try {
    /// @todo format this as the redirect does !
    if (params.port == 443) {
      return m_https_client(params);
    }
    auto ioc = net::io_context();
    int version = 11;

    DnsCacheContext context{
        .host = params.host,
        .ioc = ioc,
        .port = params.port,
        .is_tls = false,
        .force_refresh = false,
    };
    StreamGuard stream_guard(context.ioc);
    tcp_resolver &endpoints = resolve_dns_cache(context);
    stream_guard.connect(endpoints);

    http::request<http::string_body> req{params.verb, params.target, version};

    req.set(http::field::host, params.host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream_guard.get<tcp_stream>(), req); // check no copy

    beast::flat_buffer buffer;

    http::read(stream_guard.get<tcp_stream>(), buffer,
               params.http_response); // check no copy

    if (params.http_response.base().result_int() == 308) {

      std::string_view redirect_url = params.http_response.base()["Location"];
      // paths: [host,...args]
      std::vector<std::string_view> paths;
      auto substr_start = redirect_url.find('/') + 2;
      auto substr_end =
          redirect_url.substr(substr_start, redirect_url.length()).find('/');
      std::string_view redirect_host =
          redirect_url.substr(substr_start, substr_end);

      while (redirect_url.contains("/")) {
        paths.push_back(redirect_host);
        redirect_url = redirect_url.substr(substr_start, redirect_url.length());
        substr_start = redirect_url.find('/') + 1;
        substr_end =
            redirect_url.substr(substr_start, redirect_url.length()).find('/');
        redirect_host = redirect_url.substr(substr_start, substr_end);
      }

      return 308;
    }

    return params.http_response.result_int();
  } catch (std::exception const &e) {
    std::cerr << "Error " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_FAILURE;
}

u_int HttpClient::m_https_client(const HttpRequestParams &params) {

  // io context - contains context to execute the event loop and all system
  try {

    // calls needed
    // https://app.studyraid.com/en/read/12426/401278/understanding-io-context-and-its-lifecycle
    // https://stackoverflow.com/questions/60997939/what-exacty-is-io-context
    // ssl handshake context
    net::io_context ioc;
    tcp::resolver resolver{ioc};
    DnsCacheContext context{
        .host = params.host,
        .ioc = ioc,
        .port = params.port,
        .is_tls = true,
        .force_refresh = false,
    };

    auto stream = create_ssl_stream(context);

    http::request<http::string_body> req{params.verb, params.target, 11};
    req.set(http::field::host, params.host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    for (const auto &[k, v] : params.headers) {
      req.set(k, v);
    }

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::read(stream, buffer, params.http_response);

    beast::error_code ec;

  } catch (std::exception const &e) {
    std::cerr << "HTTPS error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return params.http_response.result_int();
}

/**
 * possible race conditions here if multiple resolutions hit at the same time
 */
tcp_resolver &HttpClient::resolve_dns_cache(const DnsCacheContext &context) {

  auto const key = ResolverKey(context.host, context.port, context.is_tls);
  if (!context.force_refresh && m_cachedResolutions.contains(key)) {
    return m_cachedResolutions[key];
  }

  net::io_context &ioc = context.ioc;
  tcp::resolver resolver(ioc);
  auto const resolved_results =
      resolver.resolve(context.host, std::to_string(context.port));
  m_cachedResolutions[key] = resolved_results;

  return m_cachedResolutions[key];
}

beast::ssl_stream<beast::tcp_stream>
HttpClient::create_ssl_stream(const DnsCacheContext &context) {
  if (!context.is_tls) {
    throw std::logic_error("Cannot use ssl with basic tcp stream");
  }

  tcp_resolver &results = resolve_dns_cache(context);
  beast::ssl_stream<beast::tcp_stream> stream{context.ioc, m_ssl_ioc};
  beast::get_lowest_layer(stream).connect(results);
  std::string host_str(context.host);
  if (!SSL_set_tlsext_host_name(stream.native_handle(), host_str.c_str())) {

    throw beast::system_error(
        {static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()},
        "SNI");
  }

  stream.handshake(ssl::stream_base::client);
  return stream;
}

} // namespace quarry
