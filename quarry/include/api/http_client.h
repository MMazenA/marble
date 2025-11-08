#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;

namespace quarry {
using stream = beast::ssl_stream<beast::tcp_stream>;
using tcp_resolver =
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>;
using tcp = net::ip::tcp;
using port_type = std::uint16_t;

struct DnsCacheContext {
  const std::string_view host;
  net::io_context &ioc;
  port_type port;
  const bool is_tls;
  const bool force_refresh;
};

struct HttpRequestParams {
  std::string_view host;
  port_type port;
  std::string_view target;
  http::verb verb;
  const std::unordered_map<std::string, std::string> &headers;
  http::response<http::string_body> &http_response;
};

struct ResolverKey {
  std::string_view host;
  port_type port;
  bool is_tls;
  bool operator==(const ResolverKey &other) const {
    return other.host == host && other.port == port && other.is_tls == is_tls;
  }
};

struct ResolverKeyHasher {
  auto operator()(const ResolverKey &key) const -> size_t {
    return std::hash<std::string_view>{}(key.host) ^
           std::hash<port_type>{}(key.port) ^ std::hash<bool>{}(key.is_tls);
  }
};
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

  beast::tcp_stream create_tcp_stream(DnsCacheContext &context);
};

} // namespace quarry
#endif
