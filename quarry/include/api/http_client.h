#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include "http_types.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <string>
#include <string_view>
#include <unordered_map>

namespace quarry {
class HttpClient {
public:
  HttpClient(std::string host, port_type port);

  http::response<http::string_body>
  get(std::string_view endpoint,
      const std::unordered_map<std::string, std::string> &headers = {});

  http::response<http::string_body>
  post(std::string_view endpoint, std::string_view body = "",
       const std::unordered_map<std::string, std::string> &headers = {});

private:
  std::unordered_map<ResolverKey, tcp_resolver, ResolverKeyHasher>
      m_cachedResolutions;
  std::string m_host;
  ssl::context m_ssl_ioc;
  net::io_context m_ioc;
  port_type m_port;

  u_int m_client(const HttpRequestParams &params);
  u_int m_https_client(const HttpRequestParams &params);

  static ssl::context m_make_client_ctx();

  tcp_resolver &resolve_dns_cache(const DnsCacheContext &context);

  beast::ssl_stream<beast::tcp_stream>
  create_ssl_stream(const DnsCacheContext &context);
};

} // namespace quarry
#endif
