#ifndef MIGRATION_RUNNER_H
#define MIGRATION_RUNNER_H
#include "migration.h"

namespace quarry {
void run_migrations(quarry::IMigration &migrator);

} // namespace quarry

#endif
