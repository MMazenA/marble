#include "sql.h"

namespace quarry {

std::unique_ptr<pqxx::connection> &Sql::get_connection() {
  static std::unique_ptr<pqxx::connection> m_conn;
  if (!m_conn) {
    m_conn = std::make_unique<pqxx::connection>(
        "host=localhost port=5432 dbname=marble user=user password=password");
  }
  return m_conn;
}

pqxx::result Sql::execute(std::string_view query) {
  pqxx::work w(*get_connection());
  pqxx::result result = w.exec(query);
  w.commit();
  return result;
}

void Sql::normalize_staged_aggregates(
    std::string_view ticker, std::optional<std::string_view> request_id,
    std::optional<std::string_view> display_name,
    std::optional<std::string_view> source) {
  pqxx::work txn(*get_connection());
  pqxx::params params;
  params.append(std::string{ticker});
  if (request_id.has_value()) {
    params.append(std::string{request_id.value()});
  } else {
    params.append();
  }
  if (display_name.has_value()) {
    params.append(std::string{display_name.value()});
  } else {
    params.append();
  }
  if (source.has_value()) {
    params.append(std::string{source.value()});
  } else {
    params.append("polygon");
  }
  txn.exec("SELECT normalize_aggregate_stage($1, $2, $3, $4);", params);
  txn.commit();
}

} // namespace quarry
