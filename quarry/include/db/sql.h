#ifndef QUARRY_SQL
#define QUARRY_SQL
#include "aggregates.h"
#include <cstddef>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include <string_view>
#include <tuple>
#include <vector>
namespace quarry {

class Sql {
private:
  static std::unique_ptr<pqxx::connection> &get_connection();

public:
  static pqxx::result execute(std::string_view query);
  static void normalize_staged_aggregates(
      std::string_view ticker,
      std::optional<std::string_view> request_id = std::nullopt,
      std::optional<std::string_view> display_name = std::nullopt,
      std::optional<std::string_view> source = std::nullopt);

  /**
   * Bulk insert of raw structured data into a staging table.
   *
   * @tparam T  Row type representing each record being uploaded.
   * @tparam N  Number of target table columns.
   *
   * @param rows        Collection of rows to insert.
   * @param table_name  Name of the staging table to insert into.
   * @param columns     List of column names mapped to fields in T.
   *
   * Example:
   *   bulk_insert<MyRow, 5>(rows, "staging_my_data", columns);
   */

  template <quarry::bulk_uploadable_c T, std::size_t N>
  static void bulk_insert(const std::vector<T> &rows,
                          const std::string &table_name,
                          const std::array<std::string, N> &columns) {
    pqxx::work txn(*get_connection());

    std::string columns_str;
    for (std::size_t i = 0; i < columns.size(); ++i) {
      if (i > 0) {
        columns_str += ',';
      }
      columns_str += columns[i];
    }
    auto pg_stream = pqxx::stream_to::raw_table(txn, table_name, columns_str);

    for (const auto &row : rows) {
      std::apply(
          [&pg_stream](auto &&...args) {
            pg_stream.write_values(std::forward<decltype(args)>(args)...);
          },
          row.to_tuple());
    }

    pg_stream.complete();
    txn.commit();
  }
};
} // namespace quarry

#endif
