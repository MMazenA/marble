#include "api/http_request_builder.h"

namespace quarry {

HttpRequestBuilder &HttpRequestBuilder::verb(http::verb verb) {
  m_verb = verb;
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::target(std::string_view target) {
  m_target = std::string{target};
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::version(int version) {
  m_version = version;
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::host(std::string_view host_name) {
  m_host_name = std::string{host_name};
  return *this;
}

HttpRequestBuilder &
HttpRequestBuilder::user_agent(std::string_view user_agent) {
  m_user_agent = std::string{user_agent};
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::headers(
    const std::unordered_map<std::string, std::string> &headers) {
  m_headers = headers;
  return *this;
}

HttpRequestBuilder &HttpRequestBuilder::body(std::string_view body) {
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
  return req;
}

} // namespace quarry
