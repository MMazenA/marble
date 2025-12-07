#include "migration.h"
// #include "migration_runner.h"
#include "print"
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

  quarry::Migration migrator;

  int latest_version = migrator.get_last_applied_version();
  migrator.apply_migrations(latest_version);

  return 0;
}
