// this should create my initial schema_migrations table 

#include <string>
#include <iostream>
int main(){
    const char *schema_migrations = "CREATE TABLE IF NOT EXISTS schema_migrations ("
                                    "id SERIAL PRIMARY KEY,"
                                    "version VARCHAR(100) NOT NULL UNIQUE,"
                                    "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";


    std::cout << schema_migrations << std::endl;

    return 0;
}