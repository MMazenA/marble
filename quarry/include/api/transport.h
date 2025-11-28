#ifndef QUARRY_API_TRANSPORT_H
#define QUARRY_API_TRANSPORT_H

#include "http_types.h"
#include "stream_guard.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace quarry {

class ITransport {
public:
  virtual ~ITransport() = default;
  virtual void connect(const tcp_resolver_results &endpoints) = 0;
  virtual void write(const http::request<http::string_body> &req) = 0;
  virtual void read(http::response<http::string_body> &resp) = 0;
};

class TcpTransport : public ITransport {
public:
  explicit TcpTransport(net::io_context &ioc);
  void connect(const tcp_resolver_results &endpoints) override;
  void write(const http::request<http::string_body> &req) override;
  void read(http::response<http::string_body> &resp) override;

private:
  StreamGuard m_guard;
};

class TlsTransport : public ITransport {
public:
  TlsTransport(std::string host, net::io_context &ioc, ssl::context &ssl_ctx);
  void connect(const tcp_resolver_results &endpoints) override;
  void write(const http::request<http::string_body> &req) override;
  void read(http::response<http::string_body> &resp) override;

private:
  StreamGuard m_guard;
};

} // namespace quarry

#endif
