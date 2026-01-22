#ifndef QUARRY_STREAM_GUARD_H
#define QUARRY_STREAM_GUARD_H

#include "http_types.h"
#include <boost/beast/core/tcp_stream.hpp>
#include <string>
#include <type_traits>
#include <variant>

namespace quarry {
//**
// Rule of 5
// Streams cannot be copied, they are associated with sockets
//  */
class StreamGuard {

public:
  explicit StreamGuard(net::io_context &ioc);
  StreamGuard(std::string, net::io_context &, ssl::context &);
  ~StreamGuard() noexcept;

  StreamGuard(StreamGuard &&other) noexcept;
  StreamGuard &operator=(StreamGuard &&) noexcept = delete;

  StreamGuard(const StreamGuard &) = delete;
  StreamGuard &operator=(const StreamGuard &) = delete;

  void connect(const tcp::resolver::results_type &);

  template <stream_type_c StreamType> StreamType &get() {
    return std::get<StreamType>(m_stream);
  }

  [[nodiscard]] bool is_ssl() const noexcept;
  void shutdown_safely() noexcept;

private:
  std::variant<tcp_stream, tls_stream> m_stream;
  ssl::context *m_tls_ctx;
  net::io_context &m_ioc;
  std::string m_host;

  void set_sni_hostname(const std::string &);

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

  template <typename T, typename TcpHandler, typename TlsHandler>
  T visit_stream(TcpHandler &tcp_handler, TlsHandler &tls_handler) {
    return std::visit(
        [&](auto &stream) -> T {
          using StreamType = std::decay_t<decltype(stream)>;
          if constexpr (std::is_same_v<StreamType, tls_stream>) {
            return tls_handler(stream);
          } else {
            return tcp_handler(stream);
          }
        },
        m_stream);
  }
};
} // namespace quarry
#endif
