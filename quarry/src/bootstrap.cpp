// this should create my initial schema_migrations table 

#include <string>
#include <iostream>
#include <pqxx/pqxx>

int main(){
    const char *schema_migrations = "CREATE TABLE IF NOT EXISTS schema_migrations ("
                                    "id SERIAL PRIMARY KEY,"
                                    "version VARCHAR(100) NOT NULL UNIQUE,"
                                    "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";


    std::cout << schema_migrations << std::endl;

    try {
        pqxx::connection c("host=localhost port=5432 dbname=postgres user=user password=password");

        pqxx::work txn(c);

        pqxx::result r = txn.exec("SELECT version()");

        for (auto row : r) {
            std::cout << row[0].c_str() << std::endl;
        }

        txn.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}