#include "migration_runner.h"
#include <catch2/catch_test_macros.hpp>

class MigrationMock : public quarry::IMigration {
public:
  void init() override { execution_logger.push_back("init called"); };
  void apply_migrations() override {
    execution_logger.push_back("apply_migrations called");
  };

public:
  std::vector<std::string> execution_logger;
};

TEST_CASE("Migration runs") {
  MigrationMock migrationMock;
  quarry::run_migrations(migrationMock);
  REQUIRE(migrationMock.execution_logger[0] == "init called");
  REQUIRE(migrationMock.execution_logger[1] == "apply_migrations called");
}
