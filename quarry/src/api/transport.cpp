#include "api/transport.h"
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

namespace quarry {

Transport::Transport(net::io_context &ioc) : m_guard(ioc) {}

Transport::Transport(std::string host, net::io_context &ioc,
                     ssl::context &ssl_ctx)
    : m_guard(std::move(host), ioc, ssl_ctx) {}

void Transport::connect(const tcp_resolver_results &endpoints) {
  m_guard.connect(endpoints);
}

void Transport::write(const http::request<http::string_body> &req) {
  if (m_guard.is_ssl()) {
    http::write(m_guard.get<tls_stream>(), req);
  } else {
    http::write(m_guard.get<tcp_stream>(), req);
  }
}

void Transport::read(http::response<http::string_body> &resp) {
  beast::flat_buffer buffer;
  if (m_guard.is_ssl()) {
    http::read(m_guard.get<tls_stream>(), buffer, resp);
  } else {
    http::read(m_guard.get<tcp_stream>(), buffer, resp);
  }
}

} // namespace quarry
