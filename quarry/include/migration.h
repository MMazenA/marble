#ifndef SQL_HANDLER_H
#define SQL_HANDLER_H

#include <iostream>
#include <memory>
#include <pqxx/pqxx>
namespace fs = std::filesystem;
namespace quarry {

class Migration {

public:
  Migration() {
    // setup migration table
    constexpr char *m_schema_migrations_table =
        (char *)"CREATE TABLE IF NOT EXISTS schema_migrations ("
                "id SERIAL PRIMARY KEY,"
                "version VARCHAR(100) NOT NULL UNIQUE,"
                "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    m_conn = std::make_unique<pqxx::connection>(
        "host=localhost port=5432 dbname=postgres user=user password=password");
    execute_migration_script(m_schema_migrations_table);

    // setup files to iterate go through
    const fs::path current_file_path = __FILE__;
    const fs::path quarry_directory =
        current_file_path.parent_path().parent_path();
    const fs::path sql_migrations_dir = quarry_directory / "config/sql";

    for (const fs::directory_entry &entry :
         fs::directory_iterator(sql_migrations_dir)) {
      if (fs::is_regular_file(entry)) {
        std::cout << entry.path() << std::endl;
      }
    }
  };

  pqxx::result execute_migration_script(const char *sql_migration_script) {
    try {
      /**
       * @todo
       * This needs to be moved to an env file
       * .gitignore it and then also add it to a .env.example
       */

      pqxx::work write_tsx(*m_conn);
      pqxx::result r = write_tsx.exec(sql_migration_script);
      write_tsx.commit();
      return r;

    } catch (std::exception const &e) {
      std::cerr << e.what() << std::endl;
      throw e;
    }
  }

private:
  std::unique_ptr<pqxx::connection> m_conn;
  std::vector<std::string> sql_migration_file_paths;
};

} // namespace quarry

#endif
