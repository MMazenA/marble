#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <span>
#include <string>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

namespace quarry {
using tcp = net::ip::tcp;
class httpClient {
public:
  httpClient(
      std::string host, std::string port,
      std::unordered_map<std::string, std::string> persistent_headers = {});

  http::response<http::dynamic_body>
  get(const std::string_view endpoint,
      const std::unordered_map<std::string, std::string> headers = {});

  http::response<http::dynamic_body>
  post(const std::string_view endpoint, const std::string_view body = "",
       std::unordered_map<std::string, std::string> headers = {});

private:
  int m_requests_made; // gonna pass on this for now lol, need a mutex to
                       // increment this but don't want each request to need to
                       // wait for one another

  std::string m_host;
  std::string m_port;

  std::unordered_map<std::string, std::string> m_persistent_headers;

  int m_client(const std::string_view host, const std::string_view port,
               const std::string_view target, http::verb verb,
               const std::unordered_map<std::string, std::string> &headers,
               http::response<http::dynamic_body> &http_response);
  // int m_https_client(
  //     const std::string_view host, const std::string_view port,
  //     const std::string_view target, http::verb verb,
  //     const std::unordered_map<std::string, std::string> &headers,
  //     http::response<http::dynamic_body> &http_response);
};
} // namespace quarry
#endif
