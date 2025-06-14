#include "migration.h"

namespace quarry {
void run_migrations(quarry::IMigration &migrator) {
  migrator.init();
  migrator.apply_migrations();
}

} // namespace quarry
