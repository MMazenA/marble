#ifndef QUARRY_API_TRANSPORT_POOL_H
#define QUARRY_API_TRANSPORT_POOL_H

#include "api/transport.h"
#include "dns_cache.h"
#include "http_types.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http.hpp>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <print>
#include <vector>

namespace quarry {

/**
 * @brief A thread safe pool for managing Transport/Stream objects optimized for
 * stream reuse on TLS streams
 */
class TransportPool {

public:
  // tls
  TransportPool(std::uint16_t max_connections, const std::string &host,
                net::io_context &ioc, ssl::context &ssl_ctx,
                const tcp_resolver_results &endpoints);

  TransportPool(TransportPool &&other) noexcept;
  TransportPool &operator=(TransportPool &&other) noexcept = delete;

  TransportPool(TransportPool const &other) noexcept = delete;
  TransportPool &operator=(TransportPool const &other) = delete;

  void send_and_read(const http::request<http::string_body> &request,
                     http::response<http::string_body> &response);

private:
  std::uint16_t m_max_connections;
  std::vector<std::unique_ptr<quarry::Transport>> m_transports;
  std::vector<int> m_free_list;
  std::mutex m_free_mutex;
  std::condition_variable m_free_cv;
  tcp_resolver_results m_endpoints;
  std::string m_host;
  net::io_context &m_ioc;
  ssl::context *m_ssl_ctx;
  bool m_is_tls;

  int acquire_index();
  void restore_stream(int idx);
};

} // namespace quarry

#endif
