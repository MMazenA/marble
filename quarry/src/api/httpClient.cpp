#include "api/http_client.h"
#include <format>
#include <iostream>

quarry::HttpClient::HttpClient(
    std::string host, port_type port,
    const std::unordered_map<std::string, std::string> &persistent_headers)
    : m_host(std::move(host)), m_port(port), m_ssl_ioc(m_make_client_ctx()) {}

ssl::context quarry::HttpClient::m_make_client_ctx() {
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
http::response<http::string_body> quarry::HttpClient::get(
    const std::string_view endpoint,
    const std::unordered_map<std::string, std::string> &headers) {
  /// @todo who should own this response
  /// I feel like maybe this should be a unique pointer passed in by ref, that
  /// way this function has no direct overhead, but what difference does that
  /// make
  http::response<http::string_body> response;

  u_int response_code =
      m_client(m_host, m_port, endpoint, http::verb::get, headers, response);

  if (response_code != 200) {
    throw std::runtime_error(std::format("HTTP Error code: {}", response_code));
  }

  /// this is returning a copy, i need to follow above @todo !!
  return response;
};

http::response<http::string_body> quarry::HttpClient::post(
    const std::string_view endpoint, const std::string_view body,
    const std::unordered_map<std::string, std::string> &headers) {
  http::response<http::string_body> response;
  u_int response_code =
      m_client(m_host, m_port, endpoint, http::verb::post, headers, response);

  if (response_code != 200) {
    throw std::runtime_error(std::format("HTTP Error code: {}", response_code));
  }

  return response;
};

/// @brief Primary client for connecting to endpoints
/// @param host www.example.com
/// @param port 80
/// @param target /endpoint
/// @param headers key:value dict of headers
/// @param http_response boost http object to save response
/// @return https response code
u_int quarry::HttpClient::m_client(
    const std::string_view host, const port_type port,
    const std::string_view target, http::verb verb,
    const std::unordered_map<std::string, std::string> &headers,
    http::response<http::string_body> &http_response) {
  try {
    /// @todo format this as the redirect does !
    if (port == 443) {
      return m_https_client(host, port, target, verb, headers, http_response);
    }
    auto ioc = net::io_context();
    int version = 11;

    quarry::DnsCacheContext context{
        .host = host,
        .ioc = ioc,
        .port = port,
        .is_tls = false,
        .force_refresh = false,
    };
    auto stream = create_tcp_stream(context);

    http::request<http::string_body> req{verb, target, version};

    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    beast::flat_buffer buffer;

    http::read(stream, buffer, http_response);

    if (http_response.base().result_int() == 308) {

      std::string_view redirect_url = http_response.base()["Location"];
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

    beast::error_code error_code;

    // NOLINTNEXTLINE(bugprone-unused-return-value, cert-err33-c)
    stream.socket().shutdown(tcp::socket::shutdown_both, error_code);

    if (error_code && error_code != beast::errc::not_connected) {
      throw beast::system_error{error_code};
    }

    return http_response.result_int();
  } catch (std::exception const &e) {
    std::cerr << "Error " << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_FAILURE;
}

u_int quarry::HttpClient::m_https_client(
    const std::string_view host, const port_type port,
    const std::string_view target, http::verb verb,
    const std::unordered_map<std::string, std::string> &headers,
    http::response<http::string_body> &http_response) {

  // io context - contains context to execute the event loop and all system
  try {

    // calls needed
    // https://app.studyraid.com/en/read/12426/401278/understanding-io-context-and-its-lifecycle
    // https://stackoverflow.com/questions/60997939/what-exacty-is-io-context
    // ssl handshake context
    net::io_context ioc;
    tcp::resolver resolver{ioc};
    quarry::DnsCacheContext context{
        .host = host,
        .ioc = ioc,
        .port = port,
        .is_tls = true,
        .force_refresh = false,
    };

    auto stream = create_ssl_stream(context);

    http::request<http::string_body> req{verb, target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    for (const auto &[k, v] : headers) {
      req.set(k, v);
    }

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::read(stream, buffer, http_response);

    beast::error_code ec;

    // NOLINTNEXTLINE(bugprone-unused-return-value, cert-err33-c)
    stream.shutdown(ec);
    if (ec == net::error::eof || ec == net::ssl::error::stream_truncated) {
      ec = {};
    }
    if (ec) {
      throw beast::system_error{ec};
    }
  } catch (std::exception const &e) {
    std::cerr << "HTTPS error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return http_response.result_int();
}

/**
 * possible race conditions here if multiple resolutions hit at the same time
 */
quarry::tcp_resolver &
quarry::HttpClient::resolve_dns_cache(const DnsCacheContext &context) {

  auto const key =
      quarry::ResolverKey(context.host, context.port, context.is_tls);
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
