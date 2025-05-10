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
  Migration() {
    m_conn = std::make_unique<pqxx::connection>(
        "host=localhost port=5432 dbname=postgres user=user password=password");
  };

  void init() override { m_setup_migration_table(); }

  void apply_migrations() override {
    file_map version_file_map;
    const auto resolved_migrations = m_resolve_migrations();

    for (const auto [version, path] : m_files_to_apply(resolved_migrations)) {
      m_execute_migration_script(
          "INSERT INTO schema_migrations (version) VALUES ($1)", version);
      std::println("Applied Migration {}", version);
    }
  }

private:
  void m_setup_migration_table() {
    constexpr std::string_view m_schema_migrations_table_sql =
        "CREATE TABLE IF NOT EXISTS schema_migrations ("
        "id SERIAL PRIMARY KEY,"
        "version VARCHAR(100) NOT NULL UNIQUE,"
        "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    m_execute_migration_script(m_schema_migrations_table_sql);
  }

  file_map &
  m_files_to_apply(const std::unordered_set<std::string> &already_resolved) {
    static file_map version_path_map;
    const fs::path current_file_path = __FILE__;
    const fs::path quarry_directory =
        current_file_path.parent_path().parent_path();
    const fs::path sql_migrations_dir = quarry_directory / "config/sql";
    const std::regex pattern(R"(V(\d+)__.+\.sql)");
    for (const fs::directory_entry &file_path :
         fs::directory_iterator(sql_migrations_dir)) {
      if (fs::is_regular_file(file_path)) {
        std::string file_name = file_path.path().filename().string();
        std::smatch match;
        std::regex_match(file_name, match, pattern);
        std::string version_str = match[1];
        if (!version_str.size() || already_resolved.count(version_str)) {
          continue;
        }
        version_path_map[version_str] = file_path.path();
      }
    }

    return version_path_map;
  }

  std::unordered_set<std::string> &m_resolve_migrations() {
    static std::unordered_set<std::string> resolved_set;
    pqxx::result select =
        m_execute_migration_script("SELECT version FROM schema_migrations;");

    for (const pqxx::row &row : select) {
      latest_resolved = std::max(latest_resolved,
                                 static_cast<std::size_t>(*row.at(0).c_str()));
      resolved_set.insert(row.at(0).c_str());
    }

    return resolved_set;
  }

  pqxx::result m_execute_migration_script(const std::string_view &mig_script,
                                          const pqxx::params &p) {
    pqxx::work write_tsx(*m_conn);
    pqxx::result result = write_tsx.exec(mig_script, p);
    write_tsx.commit();
    return result;
  }

  pqxx::result m_execute_migration_script(const std::string_view &mig_script) {
    pqxx::work write_tsx(*m_conn);
    pqxx::result result = write_tsx.exec(mig_script);
    write_tsx.commit();

    return result;
  }

private:
  std::unique_ptr<pqxx::connection> m_conn;
  std::size_t latest_resolved = 0;
};

} // namespace quarry

#endif
