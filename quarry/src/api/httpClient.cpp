#include "api/http_client.h"

quarry::httpClient::httpClient(
    std::string host, std::string port,
    std::unordered_map<std::string, std::string> persistent_headers)
    : m_host(host), m_port(port) {}

/// @brief `get` request to v2/endpoint/example and headers
/// @param endpoint
/// @param headers
/// @return
http::response<http::dynamic_body> quarry::httpClient::get(
    const std::string_view endpoint,
    const std::unordered_map<std::string, std::string> headers) {
  http::response<http::dynamic_body> response;

  int response_code =
      m_client(m_host, m_port, endpoint, http::verb::get, headers, response);

  if (response_code != 200) {
    std::cerr << "Invalid Response Recieved: " << response.result_int()
              << std::endl
              << response << std::endl;
    throw response.result_int();
  }

  return response;
};

http::response<http::dynamic_body>
quarry::httpClient::post(const std::string_view endpoint,
                         const std::string_view body,
                         std::unordered_map<std::string, std::string> headers) {
  http::response<http::dynamic_body> response;
  return response;
};

/// @brief
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
    http::request<http::string_body> req{verb, target, version};

    for (const auto &[key, value] : m_persistent_headers) {
      req.set(key, value);
    }

    for (const auto &[key, value] : headers) {
      req.set(key, value);
    }

    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // connects resolved IP and target,verb, and other headers
    http::write(stream, req);

    beast::flat_buffer buffer;

    http::read(stream, buffer, http_response);

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
