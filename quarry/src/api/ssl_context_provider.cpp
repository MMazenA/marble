#include "api/ssl_context_provider.h"

#ifdef _WIN32
#include <openssl/x509.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

static void load_windows_root_certs(SSL_CTX* ctx) {
  HCERTSTORE hStore = CertOpenSystemStoreW(0, L"ROOT");
  if (hStore == nullptr) {
    return;
  }

  X509_STORE* store = SSL_CTX_get_cert_store(ctx);
  PCCERT_CONTEXT pContext = nullptr;

  while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != nullptr) {
    const unsigned char* certData = pContext->pbCertEncoded;
    X509* x509 = d2i_X509(nullptr, &certData, static_cast<long>(pContext->cbCertEncoded));
    if (x509 != nullptr) {
      X509_STORE_add_cert(store, x509);
      X509_free(x509);
    }
  }

  CertCloseStore(hStore, 0);
}
#endif

namespace quarry {

[[nodiscard]] ssl::context SslContextProvider::make_client_ctx() {
  ssl::context ctx{ssl::context::tls_client};

#ifdef _WIN32
  load_windows_root_certs(ctx.native_handle());
#else
  ctx.set_default_verify_paths();
#endif

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
