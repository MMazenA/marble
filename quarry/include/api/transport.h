#ifndef QUARRY_API_TRANSPORT_H
#define QUARRY_API_TRANSPORT_H

#include "http_types.h"
#include "stream_guard.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http.hpp>

namespace quarry {

class Transport final {
public:
  explicit Transport(net::io_context &ioc);
  Transport(std::string host, net::io_context &ioc, ssl::context &ssl_ctx);

  void connect(const tcp_resolver_results &endpoints);
  void write(const http::request<http::string_body> &req);
  void read(http::response<http::string_body> &resp);

  [[nodiscard]] bool is_tls() const noexcept { return m_guard.is_ssl(); }

private:
  StreamGuard m_guard;
};

} // namespace quarry

#endif
