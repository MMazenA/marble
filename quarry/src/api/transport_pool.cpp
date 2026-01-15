#include "api/transport_pool.h"
#include "retry_policy.h"

namespace quarry {
// NOLINTNEXTLINE
constexpr int MAX_RETRY_ATTEMPTS_TRANSPORT = 3;

TransportPool::TransportPool(std::uint16_t max_connections,
                             const std::string &host, net::io_context &ioc,
                             ssl::context &ssl_ctx,
                             const tcp_resolver_results &endpoints,
                             const RetryPolicy &retry_policy)
    : m_max_connections(max_connections), m_endpoints(endpoints), m_host(host),
      m_ioc(ioc), m_ssl_ctx(&ssl_ctx), m_is_tls(true),
      m_retry_policy(retry_policy) {
  m_transports.reserve(m_max_connections);
  m_free_list.reserve(m_max_connections);
  for (int i = 0; i < m_max_connections; ++i) {
    auto t = std::make_unique<Transport>(host, ioc, ssl_ctx);
    t->connect(endpoints);
    m_transports.push_back(std::move(t));
    m_free_list.push_back(i);
  }
}

TransportPool::TransportPool(TransportPool &&other) noexcept
    : m_max_connections(other.m_max_connections),
      m_transports(std::move(other.m_transports)),
      m_free_list(std::move(other.m_free_list)),
      m_endpoints(std::move(other.m_endpoints)),
      m_host(std::move(other.m_host)), m_ioc(other.m_ioc),
      m_ssl_ctx(other.m_ssl_ctx), m_is_tls(other.m_is_tls),
      m_retry_policy(other.m_retry_policy) {};

void TransportPool::send_and_read(
    const http::request<http::string_body> &request,
    http::response<http::string_body> &response) {

  int idx = acquire_index();

  auto release_on_exit = [&]() {
    std::lock_guard<std::mutex> lock(m_free_mutex);
    m_free_list.push_back(idx);
    m_free_cv.notify_one();
  };

  // noexcept, guarantees release
  for (int attempt = 0; attempt < m_retry_policy.get_max_attempts();
       ++attempt) {
    quarry::Transport &transport = *m_transports[idx];
    if (auto code = transport.write_and_read(request, response);
        code != 200 && m_retry_policy.should_retry(code)) {
      m_retry_policy.wait(attempt);
      restore_stream(idx);
      continue;
    }
    break; // exit loop on non-retry
  }

  release_on_exit();
}

void TransportPool::restore_stream(int idx) {
  auto &transport = *m_transports[idx];
  transport.shut_down();
  if (m_is_tls) {
    auto new_transport = std::make_unique<Transport>(m_host, m_ioc, *m_ssl_ctx);
    new_transport->connect(m_endpoints);
    m_transports[idx] = std::move(new_transport);
  } else {
    auto new_transport = std::make_unique<Transport>(m_ioc);
    new_transport->connect(m_endpoints);
    m_transports[idx] = std::move(new_transport);
  }
}

int TransportPool::acquire_index() {
  std::unique_lock<std::mutex> lock(m_free_mutex);
  m_free_cv.wait(lock, [&]() { return !m_free_list.empty(); });
  int idx = m_free_list.back();
  m_free_list.pop_back();
  return idx;
}

} // namespace quarry
