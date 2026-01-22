#include "api/http_request_builder.h"

namespace quarry {

HttpRequestBuilder &HttpRequestBuilder::verb(http::verb verb) noexcept {
  m_verb = verb;
  return *this;
}

HttpRequestBuilder &
HttpRequestBuilder::target(std::string_view target) noexcept {
  m_target = std::string{target};
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::version(int version) noexcept {
  m_version = version;
  return *this;
}

HttpRequestBuilder &
HttpRequestBuilder::host(std::string_view host_name) noexcept {
  m_host_name = std::string{host_name};
  return *this;
}

HttpRequestBuilder &
HttpRequestBuilder::user_agent(std::string_view user_agent) noexcept {
  m_user_agent = std::string{user_agent};
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::keep_alive(bool keep_alive) noexcept {
  m_keep_alive = keep_alive;
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::headers(
    const std::unordered_map<std::string, std::string> &headers) noexcept {
  m_headers = headers;
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::body(std::string_view body) noexcept {
  m_body = std::string{body};
  return *this;
}

http::request<http::string_body> HttpRequestBuilder::build() const {
  http::request<http::string_body> req{m_verb, m_target, m_version};
  if (!m_host_name.empty()) {
    req.set(http::field::host, m_host_name);
  }
  if (!m_user_agent.empty()) {
    req.set(http::field::user_agent, m_user_agent);
  }
  for (const auto &kv : m_headers) {
    req.set(kv.first, kv.second);
  }
  if (!m_body.empty()) {
    req.body() = m_body;
    req.prepare_payload();
  }
  if (m_keep_alive) {
    req.set(http::field::connection, "keep-alive");
    req.keep_alive(true);
  }
  return req;
}

} // namespace quarry
