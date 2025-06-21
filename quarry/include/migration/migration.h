#ifndef SQL_HANDLER_H
#define SQL_HANDLER_H

#include <iostream>
#include <memory>
#include <print>
#include <regex>
#include <unordered_set>

#include <pqxx/pqxx>
namespace fs = std::filesystem;

namespace quarry {

/**
 * Migration interface for testing
 */
class IMigration {
public:
  virtual ~IMigration() = default;
  virtual void init() = 0;
  virtual void apply_migrations() = 0;
};

class Migration : public IMigration {

public:
  using file_map = std::unordered_map<std::string, fs::path>;

public:
  Migration();

  void init() override;
  void apply_migrations() override;

private:
  void m_setup_migration_table();

  file_map &
  m_files_to_apply(const std::unordered_set<std::string> &already_resolved);

  std::unordered_set<std::string> &m_resolve_migrations();

  pqxx::result m_execute_migration_script(const std::string_view &mig_script,
                                          const pqxx::params &p);

  pqxx::result m_execute_migration_script(const std::string_view &mig_script);

private:
  std::unique_ptr<pqxx::connection> m_conn;
  std::size_t latest_resolved = 0;
};

} // namespace quarry

#endif
