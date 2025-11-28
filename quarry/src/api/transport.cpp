#include "api/transport.h"
#include <boost/beast/http.hpp>

namespace quarry {

/**
 * tcp
 */
TcpTransport::TcpTransport(net::io_context &ioc) : m_guard(ioc) {}

void TcpTransport::connect(const tcp_resolver_results &endpoints) {
  m_guard.connect(endpoints);
}

void TcpTransport::write(const http::request<http::string_body> &req) {
  http::write(m_guard.get<tcp_stream>(), req);
}

void TcpTransport::read(http::response<http::string_body> &resp) {
  beast::flat_buffer buffer;
  http::read(m_guard.get<tcp_stream>(), buffer, resp);
}

/**
 * tls
 */
TlsTransport::TlsTransport(std::string host, net::io_context &ioc,
                           ssl::context &ssl_ctx)
    : m_guard(std::move(host), ioc, ssl_ctx) {}

void TlsTransport::connect(const tcp_resolver_results &endpoints) {
  m_guard.connect(endpoints);
}

void TlsTransport::write(const http::request<http::string_body> &req) {
  http::write(m_guard.get<tls_stream>(), req);
}

void TlsTransport::read(http::response<http::string_body> &resp) {
  beast::flat_buffer buffer;
  http::read(m_guard.get<tls_stream>(), buffer, resp);
}

} // namespace quarry
