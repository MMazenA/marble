#include "migration_runner.h"
#include <catch2/catch_test_macros.hpp>

class MigrationMock : public quarry::IMigration {
public:
  void init() override { execution_logger.push_back("init called"); };
  void apply_migrations() override {
    execution_logger.push_back("apply_migrations called");
  };

  std::vector<std::string> execution_logger;
};

TEST_CASE("MigrationRunner") {
  SECTION("Migration runs") {
    MigrationMock migration_mock;
    quarry::run_migrations(migration_mock);
    REQUIRE(migration_mock.execution_logger[0] == "init called");
    REQUIRE(migration_mock.execution_logger[1] == "apply_migrations called");
  }
}
