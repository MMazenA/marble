#include "migration.h"
#include <pqxx/pqxx>

int main() {

  quarry::Migration migrator;

  int latest_version = migrator.get_last_applied_version();
  migrator.apply_migrations(latest_version);

  return 0;
}
