#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;

namespace quarry {

using tcp_stream = boost::beast::tcp_stream;
using tls_stream = beast::ssl_stream<beast::tcp_stream>;
using tcp_resolver_results =
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp>;
using tcp = net::ip::tcp;
using port_type = std::uint16_t;
struct DnsCacheContext {
  const std::string_view host;
  net::io_context &ioc;
  port_type port;
  const bool is_tls;
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

template <class T>
concept stream_type_c =
    std::same_as<T, tcp_stream> || std::same_as<T, tls_stream>;

} // namespace quarry

#endif
