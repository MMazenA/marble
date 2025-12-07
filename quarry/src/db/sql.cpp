#include "sql.h"

namespace quarry {

std::unique_ptr<pqxx::connection> &Sql::get_connection() {
  static std::unique_ptr<pqxx::connection> m_conn;
  if (!m_conn) {
    m_conn = std::make_unique<pqxx::connection>(
        "host=localhost port=5432 dbname=postgres user=user password=password");
  }
  return m_conn;
}

pqxx::result Sql::execute(std::string_view query) {
  pqxx::work w(*get_connection());
  pqxx::result result = w.exec(query);
  w.commit();
  return result;
}

} // namespace quarry
