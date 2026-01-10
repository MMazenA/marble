#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include "http_types.h"
#include "ssl_context_provider.h"
#include "transport_pool.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace quarry {
class HttpClient {
public:
  /**
  todo:
    - template the ssl provider, doesnt need function call overhead
    - pass in pool size
    - pass in retry policy
      - retry policy can be its own class
      - includes base retries
      - retries on certain errors
      - jitter policy
      - refactor with std::expected as well

  */
  HttpClient(std::string host, port_type port, bool is_tls = false,
             const std::function<ssl::context()> &ctx_provider =
                 SslContextProvider::make_client_ctx,
             std::optional<int> http_pool_size = std::nullopt);

  http::response<http::string_body>
  get(std::string_view endpoint,
      const std::unordered_map<std::string, std::string> &headers = {});

  http::response<http::string_body>
  post(std::string_view endpoint, std::string_view body = "",
       const std::unordered_map<std::string, std::string> &headers = {});

private:
  std::string m_host;
  ssl::context m_ssl_ioc;
  net::io_context m_ioc;

  port_type m_port;

  u_int m_client(const HttpRequestParams &params);
  u_int m_https_client(const HttpRequestParams &params);

  bool m_is_tls = false;
  std::optional<TransportPool> m_transport_pool_tls;
};

} // namespace quarry
#endif
