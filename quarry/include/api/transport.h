#ifndef QUARRY_API_TRANSPORT_H
#define QUARRY_API_TRANSPORT_H

#include "base_endpoint.h"
#include "dns_cache.h"
#include "http_types.h"
#include "stream_guard.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http.hpp>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <print>
#include <vector>

namespace quarry {

class Transport {
public:
  explicit Transport(net::io_context &ioc);
  Transport(std::string host, net::io_context &ioc, ssl::context &ssl_ctx);

  void connect(const tcp_resolver_results &endpoints);
  void write(const http::request<http::string_body> &req);
  void read(http::response<http::string_body> &resp);
  bool cycle(const http::request<http::string_body> &req,
             http::response<http::string_body> &resp) noexcept;
  [[nodiscard]] bool is_open();
  [[nodiscard]] bool is_tls() const noexcept { return m_guard.is_ssl(); }
  void shut_down() noexcept;

private:
  StreamGuard m_guard;
};

/**
 * Stream Pooling to read and write from the same stream to skip handshakes
 */
class TransportPool {

public:
  // tls
  TransportPool(std::uint16_t max_connections, const std::string &host,
                net::io_context &ioc, ssl::context &ssl_ctx,
                const tcp_resolver_results &endpoints)
      : m_max_connections(max_connections), m_endpoints(endpoints),
        m_host(host), m_ioc(ioc), m_ssl_ctx(&ssl_ctx), m_is_tls(true) {
    m_transports.reserve(m_max_connections);
    m_free_list.reserve(m_max_connections);
    for (int i = 0; i < m_max_connections; ++i) {
      auto t = std::make_unique<Transport>(host, ioc, ssl_ctx);
      t->connect(endpoints);
      m_transports.push_back(std::move(t));
      m_free_list.push_back(i);
    }
  };

  // tcp
  TransportPool(std::uint16_t max_connections, net::io_context &ioc,
                const tcp_resolver_results &endpoints)
      : m_max_connections(max_connections), m_endpoints(endpoints), m_ioc(ioc),
        m_ssl_ctx(nullptr), m_is_tls(false) {

    m_transports.reserve(m_max_connections);
    m_free_list.reserve(m_max_connections);
    for (int i = 0; i < m_max_connections; ++i) {
      auto t = std::make_unique<Transport>(ioc);
      t->connect(endpoints);
      m_transports.push_back(std::move(t));
      m_free_list.push_back(i);
    }
  };

  void send_and_read(const http::request<http::string_body> &request,
                     http::response<http::string_body> &response) {

    int idx = acquire_index();

    auto release_on_exit = [&]() {
      std::lock_guard<std::mutex> lock(m_free_mutex);
      m_free_list.push_back(idx);
      m_free_cv.notify_one();
    };

    const int max_attempts = 3;
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
      quarry::Transport &transport = *m_transports[idx];
      if (transport.cycle(request, response)) {
        break;
      }
      restore_stream(idx);
    }

    release_on_exit();
  }

  void restore_stream(int idx) {
    std::println("restoring stream idx: {}", idx);
    auto &transport = *m_transports[idx];
    transport.shut_down();
    if (m_is_tls) {
      auto new_transport =
          std::make_unique<Transport>(m_host, m_ioc, *m_ssl_ctx);
      new_transport->connect(m_endpoints);
      m_transports[idx] = std::move(new_transport);
    } else {
      auto new_transport = std::make_unique<Transport>(m_ioc);
      new_transport->connect(m_endpoints);
      m_transports[idx] = std::move(new_transport);
    }
  }

private:
  std::uint16_t m_max_connections;
  std::vector<std::unique_ptr<quarry::Transport>> m_transports;
  std::vector<int> m_free_list;
  std::mutex m_free_mutex;
  std::condition_variable m_free_cv;
  const tcp_resolver_results &m_endpoints;
  std::string m_host;
  net::io_context &m_ioc;
  ssl::context *m_ssl_ctx;
  bool m_is_tls;

  int acquire_index() {
    std::unique_lock<std::mutex> lock(m_free_mutex);
    m_free_cv.wait(lock, [&]() { return !m_free_list.empty(); });
    int idx = m_free_list.back();
    m_free_list.pop_back();
    return idx;
  }
};

} // namespace quarry

#endif
