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
  MigrationMock MigrationMock;
  quarry::run_migrations(MigrationMock);
  REQUIRE(MigrationMock.execution_logger[0] == "init called");
  REQUIRE(MigrationMock.execution_logger[1] == "apply_migrations called");
}
