// this should create my initial schema_migrations table

#include "migration_runner.h"
#include <pqxx/pqxx>

void print_sql_response(const pqxx::result &r) {
  // print column names
  constexpr auto row_format = "{0:20}";

  for (const auto &row : r) {
    for (const auto &col : row) {
      std::print(row_format, col.name());
    }
  }
}

int main() {

  std::unique_ptr<quarry::IMigration> migrator =
      std::make_unique<quarry::Migration>();
  quarry::run_migrations(*migrator);

  return 0;
}
