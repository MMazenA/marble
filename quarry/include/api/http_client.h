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
#include <stdexcept>
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
class HttpClient { // 176
public:
  HttpClient(std::string host, port_type port,
             const std::unordered_map<std::string, std::string>
                 &persistent_headers = {});

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

  u_int m_client(std::string_view host, port_type port, std::string_view target,
                 http::verb verb,
                 const std::unordered_map<std::string, std::string> &headers,
                 http::response<http::string_body> &http_response);
  u_int
  m_https_client(std::string_view host, port_type port, std::string_view target,
                 http::verb verb,
                 const std::unordered_map<std::string, std::string> &headers,
                 http::response<http::string_body> &http_response);

  static ssl::context m_make_client_ctx();

  tcp_resolver &resolve_dns_cache(DnsCacheContext &context);

  beast::ssl_stream<beast::tcp_stream>
  create_ssl_stream(DnsCacheContext context) {
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
    return std::move(stream);
  };

  beast::tcp_stream create_tcp_stream(DnsCacheContext &context) {
    if (context.is_tls) {
      throw std::logic_error("Cannot use basic tcp stream with tls");
    }

    tcp_resolver &results = resolve_dns_cache(context);
    beast::tcp_stream stream(m_ioc);
    stream.connect((results));
    return std::move(stream);
  }
};

} // namespace quarry
#endif
