#include "api/ssl_context_provider.h"
#include <iostream>

namespace quarry {

[[nodiscard]] ssl::context SslContextProvider::make_client_ctx() {
  ssl::context ctx{ssl::context::tls_client};
  ctx.set_default_verify_paths();
  ctx.set_verify_mode(ssl::verify_peer);

  SSL_CTX_set_session_cache_mode(ctx.native_handle(), SSL_SESS_CACHE_CLIENT);
  SSL_CTX_sess_set_cache_size(ctx.native_handle(), 128);

  return ctx;
}

/**
 * @warning Only used for unit testing purposes, do not use on a real request
 */
[[nodiscard]] ssl::context SslContextProvider::make_insecure_client_ctx() {
  ssl::context ctx{ssl::context::tls_client};
  ctx.set_verify_mode(ssl::verify_none);

  SSL_CTX_set_session_cache_mode(ctx.native_handle(), SSL_SESS_CACHE_CLIENT);
  SSL_CTX_sess_set_cache_size(ctx.native_handle(), 128);

  return ctx;
}

} // namespace quarry
