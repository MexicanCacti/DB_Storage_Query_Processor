#include <iostream>
#include "database.hpp"
#include <string>
#include <filesystem>


int main()
{
    std::filesystem::path databaseDir = "./database_data";
    std::string tableName = "students";
    Database database(databaseDir, 10);
    Schema schema({
    {"id", FieldType::INT64},
    {"name", FieldType::STRING32}
    });

    database.createTable(tableName, schema);

    std::cout << "HELLO!\n";

    return 0;
}