#include "api/http_client.h"

quarry::httpClient::httpClient(
    std::string host, std::string port,
    std::unordered_map<std::string, std::string> persistent_headers)
    : m_host(host), m_port(port) {}

ssl::context quarry::httpClient::make_client_ctx() {
  ssl::context ctx{ssl::context::tls_client};

  ctx.set_default_verify_paths();

  ctx.set_verify_mode(ssl::verify_peer);

  return ctx;
}

/// @brief `get` request to v2/endpoint/example and headers
/// @param endpoint
/// @param headers
/// @return
http::response<http::dynamic_body> quarry::httpClient::get(
    const std::string_view endpoint,
    const std::unordered_map<std::string, std::string> headers) {
  /// @todo who should own this response
  /// I feel like maybe this should be a unique pointer passed in by ref, that
  /// way this function has no direct overhead, but what difference does that
  /// make
  http::response<http::dynamic_body> response;

  int response_code =
      m_client(m_host, m_port, endpoint, http::verb::get, headers, response);

  if (response_code != 200) {
    std::cerr << "Invalid Response Received: " << response_code << std::endl
              << response << std::endl;
    throw response_code;
  }

  /// this is returning a copy, i need to follow above @todo !!
  return response;
};

http::response<http::dynamic_body>
quarry::httpClient::post(const std::string_view endpoint,
                         const std::string_view body,
                         std::unordered_map<std::string, std::string> headers) {
  http::response<http::dynamic_body> response;
  int response_code =
      m_client(m_host, m_port, endpoint, http::verb::post, headers, response);

  if (response_code != 200) {
    std::cerr << "Invalid Response Received: " << response_code << std::endl
              << response << std::endl;
    throw response_code;
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
int quarry::httpClient::m_client(
    const std::string_view host, const std::string_view port,
    const std::string_view target, http::verb verb,
    const std::unordered_map<std::string, std::string> &headers,
    http::response<http::dynamic_body> &http_response) {
  try {
    /// @todo format this as the reidredct does !
    if (port == "443") {
      return m_https_client(host, port, target, verb, headers, http_response);
    }
    net::io_context ioc;
    tcp::resolver resolver(ioc);   // used to dispatch requests
    beast::tcp_stream stream(ioc); // TCP stream socket
    int version = 11;

    // retrieves list of resolved ip:port pairs for the corresponding
    // host:port
    auto const resolved_results = resolver.resolve(host, port);

    // connect to the IP returned from the DNS lookup (goes through the list)
    stream.connect(resolved_results);

    // setting up the request object
    http::request<http::string_body> req;
    req.target(target);
    req.method(verb);
    req.version(version);

    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // connects resolved IP and target,verb, and other headers
    http::write(stream, req);

    beast::flat_buffer buffer;

    http::read(stream, buffer, http_response);

    if (http_response.base().result_int() == 308) {

      std::string_view redirect_url = http_response.base()["Location"];
      // paths: [host,...args]
      std::vector<std::string_view> paths;
      auto substr_start = redirect_url.find("/") + 2;
      auto substr_end =
          redirect_url.substr(substr_start, redirect_url.length()).find("/");
      std::string_view redirect_host =
          redirect_url.substr(substr_start, substr_end);

      while (redirect_url.contains("/")) {
        paths.push_back(redirect_host);
        redirect_url = redirect_url.substr(substr_start, redirect_url.length());
        substr_start = redirect_url.find("/") + 1;
        substr_end =
            redirect_url.substr(substr_start, redirect_url.length()).find("/");
        redirect_host = redirect_url.substr(substr_start, substr_end);
      }

      return 308;
    }

    beast::error_code ec;

    // closes reading and writing side of the stream
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    if (ec && ec != beast::errc::not_connected) {
      throw beast::system_error{ec};
    }

    return http_response.result_int();
  } catch (std::exception const &e) {
    std::cerr << "Error " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_FAILURE;
}

int quarry::httpClient::m_https_client(
    const std::string_view host, const std::string_view port,
    const std::string_view target, http::verb verb,
    const std::unordered_map<std::string, std::string> &headers,
    http::response<http::dynamic_body> &http_response) {

  // io context - contains context to execute the event loop and all system
  try {

    // calls needed
    // https://app.studyraid.com/en/read/12426/401278/understanding-io-context-and-its-lifecycle
    // https://stackoverflow.com/questions/60997939/what-exacty-is-io-context
    // ssl handshake context
    net::io_context ioc;
    ssl::context ssl_ctx = make_client_ctx();
    tcp::resolver resolver{ioc};

    beast::ssl_stream<beast::tcp_stream> stream{ioc, ssl_ctx};

    auto const results = resolver.resolve(host, port);
    beast::get_lowest_layer(stream).connect(results);

    // openSSL stuff?
    std::string host_str(host);

    if (!SSL_set_tlsext_host_name(stream.native_handle(), host_str.c_str()))
      throw beast::system_error(
          {static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()},
          "SNI");

    stream.handshake(ssl::stream_base::client);

    http::request<http::string_body> req{verb, target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    for (auto &[k, v] : headers) {
      req.set(k, v);
    }

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::read(stream, buffer, http_response);

    beast::error_code ec;

    stream.shutdown(ec);
    if (ec == net::error::eof || ec == net::ssl::error::stream_truncated)
      ec = {};
    if (ec)
      throw beast::system_error{ec};
  } catch (std::exception const &e) {
    std::cerr << "HTTPS error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << http_response << std::endl;
  return http_response.result_int();
}
