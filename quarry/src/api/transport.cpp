#include "api/transport.h"
#include "http_types.h"
#include "logging.h"
#include "retry_policy.h"
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <quill/LogMacros.h>

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

/// @brief Submit a request and read the response
/// @param req
/// @param resp
/// @return bool: Status of response read/write
unsigned int
Transport::write_and_read(const http::request<http::string_body> &req,
                          http::response<http::string_body> &resp) noexcept {
  try {
    write(req);
    read(resp);
    return resp.result_int();
  } catch (const boost::system::system_error &ec) {
    auto *logger = quarry::logging::get_logger();
    LOG_INFO(logger, "Cycled read/write, eof dead stream");
    return quarry::DEAD_STREAM_ERROR_CODE;
  } catch (...) {
    auto *logger = quarry::logging::get_logger();
    LOG_ERROR(logger, "Unknown error on cycled read/write");
    return quarry::DEAD_STREAM_ERROR_CODE;
  }
}

bool Transport::is_open() {
  if (m_guard.is_ssl()) {
    const auto &tls_socket = m_guard.get<tls_stream>().lowest_layer();
    return tls_socket.is_open();
  }
  const auto &socket = m_guard.get<tcp_stream>().socket();
  return socket.is_open();
}

void Transport::shut_down() noexcept { m_guard.shutdown_safely(); }

} // namespace quarry
