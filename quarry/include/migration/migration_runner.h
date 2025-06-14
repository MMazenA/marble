#ifndef MIGRATION_RUNNER_H
#define MIGRATION_RUNNER_H
#include "migration.h"

namespace quarry {
void run_migrations(quarry::IMigration &migrator) {
  migrator.init();
  migrator.apply_migrations();
}

} // namespace quarry

#endif
