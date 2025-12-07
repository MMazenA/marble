

#include "db/migration.h"
#include <fstream>
#include <memory>
#include <pqxx/pqxx>
#include <print>
#include <regex>
#include <sstream>
#include <string>

namespace quarry {

Migration::Migration() {
  m_conn = std::make_unique<pqxx::connection>(
      "host=localhost port=5432 dbname=postgres user=user password=password");
  setup_migration_table();
};

void Migration::apply_migrations(int last_applied_version) {
  file_map files_to_apply = get_files_to_apply(last_applied_version);

  for (const auto &[version, path] : files_to_apply) {
    std::ifstream sql_file(path);
    if (!sql_file.is_open()) {
      std::println("Failed to open migration file: {}", path.string());
      continue;
    }

    std::stringstream buffer;
    buffer << sql_file.rdbuf();
    std::string sql_content = buffer.str();

    execute_script(sql_content);
    execute("INSERT INTO schema_migrations (version) VALUES ($1)", version);
    std::println("Applied Migration {}", version);
  }
}

void Migration::setup_migration_table() {
  constexpr std::string_view m_schema_migrations_table_sql =
      "CREATE TABLE IF NOT EXISTS schema_migrations ("
      "id SERIAL PRIMARY KEY,"
      "version VARCHAR(100) NOT NULL UNIQUE,"
      "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

  execute(m_schema_migrations_table_sql);
}

pqxx::result Migration::execute(std::string_view mig_script,
                                const pqxx::params &p) {
  pqxx::work write_tsx(*m_conn);
  pqxx::result result = write_tsx.exec(mig_script, p);
  write_tsx.commit();
  return result;
}

void Migration::execute_script(std::string_view sql_script) {
  pqxx::work txn(*m_conn);
  txn.exec(std::string{sql_script}).no_rows();
  txn.commit();
}

int Migration::get_last_applied_version() {
  pqxx::result select = execute("SELECT version FROM schema_migrations;");
  int latest_resolved = 0;
  for (const pqxx::row &row : select) {
    latest_resolved =
        std::max(latest_resolved, std::stoi(std::string{*row.at(0).c_str()}));
  }

  return latest_resolved;
}

Migration::file_map Migration::get_files_to_apply(int lower_bound) {
  Migration::file_map version_to_path = {};

  const fs::path current_file_path = __FILE__;
  const fs::path quarry_directory =
      current_file_path.parent_path().parent_path().parent_path();
  const fs::path sql_migrations_dir = quarry_directory / "config/sql";
  const std::regex pattern(R"(V(\d+)__.+\.sql)");

  for (const fs::directory_entry &file_path :
       fs::directory_iterator(sql_migrations_dir)) {
    if (fs::is_regular_file(file_path)) {
      std::string file_name = file_path.path().filename().string();
      std::smatch match;
      std::regex_match(file_name, match, pattern);

      int version = stoi(match[1]);
      if (version < lower_bound) {
        continue;
      }
      version_to_path[version] = file_path.path();
    }
  }

  return version_to_path;
}

} // namespace quarry
