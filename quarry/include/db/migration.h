#ifndef SQL_HANDLER_H
#define SQL_HANDLER_H

#include <filesystem>
#include <memory>
#include <pqxx/pqxx>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
namespace fs = std::filesystem;

namespace quarry {

/**
 Usage:
     int latest_version = get_last_applied_version();
     apply_migrations(latest_version);
*/
class Migration {
  using file_map = std::unordered_map<int, fs::path>;

public:
  Migration();

  void setup_migration_table();
  int get_last_applied_version();
  static file_map get_files_to_apply(int lower_bound);
  void apply_migrations(int last_applied_version);
  pqxx::result execute(std::string_view mig_script, const pqxx::params &p = {});
  void execute_script(std::string_view sql_script);

private:
  std::unique_ptr<pqxx::connection> m_conn;
};

} // namespace quarry

#endif
