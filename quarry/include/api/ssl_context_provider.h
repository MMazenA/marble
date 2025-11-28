#ifndef QUARRY_API_SSL_CONTEXT_PROVIDER_H
#define QUARRY_API_SSL_CONTEXT_PROVIDER_H

#include <boost/beast/ssl.hpp>

namespace quarry {

namespace net = boost::asio;
namespace ssl = net::ssl;
class SslContextProvider {
public:
  [[nodiscard]] static ssl::context make_client_ctx();
};

} // namespace quarry

#endif
