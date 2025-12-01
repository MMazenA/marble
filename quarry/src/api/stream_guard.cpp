#include "stream_guard.h"
#include "http_types.h"
#include <boost/asio/io_context.hpp>

namespace quarry {
StreamGuard::StreamGuard(net::io_context &ioc)
    : m_stream(std::in_place_type<tcp_stream>, ioc), m_tls_ctx(nullptr),
      m_ioc(ioc) {};

StreamGuard::StreamGuard(std::string host, net::io_context &ioc,
                         ssl::context &tls_ctx)
    : m_stream(std::in_place_type<tls_stream>, ioc, tls_ctx),
      m_tls_ctx(&tls_ctx), m_ioc(ioc), m_host(std::move(host)) {};

StreamGuard::StreamGuard(StreamGuard &&other) noexcept
    : m_stream(std::move(other.m_stream)), m_tls_ctx(other.m_tls_ctx),
      m_ioc(other.m_ioc), m_host(std::move(other.m_host)) {};

StreamGuard::~StreamGuard() noexcept { shutdown_safely(); };

// configures ssl stream and handshakes the stream
void StreamGuard::set_sni_hostname(const std::string &host_str) {
  auto tcp_handler = [&](tcp_stream &stream) {};
  auto tls_handler = [&](tls_stream &stream) {
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host_str.c_str())) {
      auto err_code = static_cast<int>(::ERR_get_error());
      auto err_category = net::error::get_ssl_category;
      throw beast::system_error({err_code, err_category()}, "SNI");
    }
  };

  visit_stream(tcp_handler, tls_handler);
}

void StreamGuard::connect(const tcp::resolver::results_type &endpoints) {
  auto tcp_handler = [&](tcp_stream &stream) { stream.connect(endpoints); };

  auto tls_handler = [&](tls_stream &stream) {
    beast::get_lowest_layer(stream).connect(endpoints);
    set_sni_hostname(m_host);
    stream.handshake(ssl::stream_base::client);
  };

  visit_stream(tcp_handler, tls_handler);
}

bool StreamGuard::is_ssl() const noexcept {
  return holds_stream_type<tls_stream>();
};

void StreamGuard::shutdown_safely() noexcept {

  auto tcp_handler = [&](tcp_stream &stream) {
    beast::error_code error_code;
    // NOLINTNEXTLINE(bugprone-unused-return-value, cert-err33-c)
    stream.socket().shutdown(tcp::socket::shutdown_both, error_code);
    stream.close();
  };
  auto tls_handler = [&](tls_stream &stream) {
    beast::error_code error_code;
    // NOLINTNEXTLINE(bugprone-unused-return-value, cert-err33-c)
    stream.shutdown(error_code);
    beast::get_lowest_layer(stream).close();
  };

  visit_stream(tcp_handler, tls_handler);
}

} // namespace quarry
