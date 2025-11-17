#ifndef QUARRY_DNS_CACHE_H
#define QUARRY_DNS_CACHE_H

#include "http_types.h"
#include <memory>
#include <shared_mutex>

namespace quarry {
class DnsCache {
public:
  static std::shared_ptr<DnsCache> global_cache();

  DnsCache() = default;

  DnsCache &operator=(const DnsCache &) = delete;
  DnsCache(const DnsCache &) = delete;

  DnsCache &operator=(DnsCache &&) noexcept;
  DnsCache(DnsCache &&) noexcept;

  ~DnsCache() = default;

  [[nodiscard]] const tcp_resolver_results &get(const DnsCacheContext &) const;

private:
  mutable std::shared_mutex m_cache_lock;
  mutable std::unordered_map<ResolverKey, tcp_resolver_results,
                             ResolverKeyHasher>
      m_cached_resolutions;
};

} // namespace quarry

#endif
