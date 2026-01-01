#include "dns_cache.h"
#include <shared_mutex>

namespace quarry {

DnsCache &DnsCache::global_cache() {
  static DnsCache singleton{};
  return singleton;
};

DnsCache &DnsCache::operator=(DnsCache &&other) noexcept {
  if (this != &other) {
    std::scoped_lock<std::shared_mutex, std::shared_mutex> lock(
        other.m_cache_lock, this->m_cache_lock);

    m_cached_resolutions = std::move(other.m_cached_resolutions);
  }
  return *this;
}

DnsCache::DnsCache(DnsCache &&other) noexcept {
  std::unique_lock<std::shared_mutex> lock(other.m_cache_lock);
  m_cached_resolutions = std::move(other.m_cached_resolutions);
}

const tcp_resolver_results &
DnsCache::get(const DnsCacheContext &context) const {

  auto const key = ResolverKey{
      .host = context.host, .port = context.port, .is_tls = context.is_tls};
  // scoped read access
  {
    std::shared_lock<std::shared_mutex> rlock(m_cache_lock);
    if (auto it = m_cached_resolutions.find(key);
        it != m_cached_resolutions.end()) {
      return it->second;
    }
  }

  // scoped write access
  {
    std::unique_lock<std::shared_mutex> wlock(m_cache_lock);
    if (auto it = m_cached_resolutions.find(key);
        it != m_cached_resolutions.end()) {
      return it->second;
    }

    net::io_context &ioc = context.ioc;
    tcp::resolver resolver(ioc);
    auto const resolved_results =
        resolver.resolve(context.host, std::to_string(context.port));

    auto [it, is_inserted] =
        m_cached_resolutions.try_emplace(key, resolved_results);
    return it->second;
  }
}

} // namespace quarry
