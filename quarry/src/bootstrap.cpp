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

  quarry::Migration *sql = new quarry::Migration();

  //   constexpr char *schema_migrations_table =
  //       "CREATE TABLE IF NOT EXISTS schema_migrations ("
  //       "id SERIAL PRIMARY KEY,"
  //       "version VARCHAR(100) NOT NULL UNIQUE,"
  //       "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

  //   std::cout << schema_migrations_table << std::endl;
  //   execute_migration_script(schema_migrations_table);

  //   const char *schema_migrations_select = "SELECT * FROM
  //   schema_migrations;";

  //   pqxx::result r = execute_migration_script(schema_migrations_select);
  //   print_sql_response(r);

  return 0;
}
