// this should create my initial schema_migrations table 

#include <string>
#include <iostream>
#include <pqxx/pqxx>


std::string SQL_FILES_DIRECTORY = "./config/sql";

void execute_migration_script(const char *migration_script){

    try {
        /**
         * @todo
         * This needs to be moved to an env file
         * .gitignore it and then also add it to a .env.example
         */
        pqxx::connection c("host=localhost port=5432 dbname=postgres user=user password=password");

        {
            pqxx::work txn(c);
            pqxx::result r = txn.exec(migration_script);
            txn.commit();
        }

        pqxx::work txn(c);
        pqxx::result r = txn.exec("SELECT version()");

        for (auto row : r) {
            std::cout << row[0].c_str() << std::endl;
        }

        txn.commit();
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}



int main(){

    /**
     * @todo
     * 
     * Want to load these in dynamically from config/sql/*.sql
     * Load a single ordered file, VXX__name.sql
     * Execute him
     * Garbage collection
     * Start the next one 
     */

    const char *schema_migrations = "CREATE TABLE IF NOT EXISTS schema_migrations ("
                                    "id SERIAL PRIMARY KEY,"
                                    "version VARCHAR(100) NOT NULL UNIQUE,"
                                    "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";


    std::cout << schema_migrations << std::endl;


    return 0;
}
