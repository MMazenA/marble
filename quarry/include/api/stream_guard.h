#ifndef STREAM_GUARD_H
#define STREAM_GUARD_H

#include "http_types.h"
#include <boost/beast/core/tcp_stream.hpp>
#include <string>
#include <type_traits>
#include <variant>

namespace quarry {
class StreamGuard {

public:
  explicit StreamGuard(net::io_context &ioc);
  StreamGuard(std::string, net::io_context &, ssl::context &);
  ~StreamGuard() noexcept;

  StreamGuard(StreamGuard &&) noexcept = delete;
  StreamGuard &operator=(StreamGuard &&) noexcept = delete;

  StreamGuard(const StreamGuard &) = delete;
  StreamGuard &operator=(const StreamGuard &) = delete;

  void connect(const tcp::resolver::results_type &);
  void set_sni_hostname(const std::string &);

  template <stream_type_c StreamType> StreamType &get() {
    return std::get<StreamType>(m_stream);
  }

  [[nodiscard]] bool is_ssl() const noexcept;

private:
  std::variant<tcp_stream, tls_stream> m_stream;
  ssl::context *m_tls_ios;
  net::io_context &m_ioc;
  std::string m_host;

  void shutdown_safely() noexcept;

  template <stream_type_c StreamType>
  [[nodiscard]] constexpr bool holds_stream_type() const {
    return std::holds_alternative<StreamType>(m_stream);
  }

  template <typename TcpHandler, typename TlsHandler>
  void visit_stream(TcpHandler &tcp_handler, TlsHandler &tls_handler) {
    // https://medium.com/@weidagang/modern-c-std-variant-and-std-visit-3c16084db7dc
    std::visit(
        [&](auto &stream) {
          using StreamType = std::decay_t<decltype(stream)>;
          if constexpr (std::is_same_v<StreamType, tls_stream>) {
            tls_handler(stream);
          } else if constexpr (std::is_same_v<StreamType, tcp_stream>) {
            tcp_handler(stream);
          }
        },
        m_stream);
  }
};
} // namespace quarry
#endif
