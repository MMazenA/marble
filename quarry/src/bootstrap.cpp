// this should create my initial schema_migrations table

#include <iostream>
#include <migration.h>
#include <pqxx/pqxx>
#include <string>

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

  /**
   * @todo
   *
   * Want to load these in dynamically from config/sql/ .sql
   * Load a single ordered file, VXX__name.sql
   * Execute him
   * Garbage collection
   * Start the next one
   */

  quarry::Migration migrator;
  migrator.init();
  migrator.apply_migrations();

  return 0;
}
