#include <filesystem>
#include <memory>
#include <vector>
#include <stdexcept>
#include <vector>
#include <windows.h>
#include <iostream>

#include "executor.hpp"
#include "expression.hpp"
#include "column_expression.hpp"
#include "constant_expression.hpp"
#include "addition_expression.hpp"
#include "equal_expression.hpp"
#include "greater_than_expression.hpp"
#include "greater_than_equal_expression.hpp"
#include "and_expression.hpp"

#include "values_executor.hpp"
#include "insert_executor.hpp"
#include "sequential_scan_executor.hpp"
#include "projection_executor.hpp"
#include "filter_executor.hpp"
#include "nested_loop_join_executor.hpp"
#include "database.hpp"


std::string directoryString = "./storage";

namespace fs = std::filesystem;

bool compareRID(RID& r1, RID& r2)
{
    if(r1.pageID.tableID != r2.pageID.tableID || r1.pageID.pageNumber != r2.pageID.pageNumber || r1.tupleSlot != r2.tupleSlot) return false;
    return true;
}

std::filesystem::path getExecutablePath()
{
    std::wstring buffer(32768, L'\0');

    DWORD length = GetModuleFileNameW(
        nullptr,
        buffer.data(),
        static_cast<DWORD>(buffer.size())
    );

    if(length == 0 || length >= buffer.size())
        throw std::runtime_error("Failed to retrieve executable path");

    buffer.resize(length);

    return std::filesystem::path(buffer);
}

bool step_one(std::shared_ptr<Database> database, std::vector<std::pair<Tuple, RID>>& studentRIDS);

bool step_two(std::shared_ptr<Database> database, std::vector<std::pair<Tuple, RID>>& studentRIDS);

bool step_three(std::shared_ptr<Database> database, std::vector<std::pair<Tuple, RID>>& studentRIDS);

// bool step_four(std::shared_ptr<Database> database);

// bool step_five(std::shared_ptr<Database>database);

// bool step_six(std::shared_ptr<Database>database);

// bool step_seven(std::shared_ptr<Database>database);

Schema studentSchema({
    Field{"id", FieldType::INT64},
    Field{"name", FieldType::STRING32},
    Field{"dept", FieldType::STRING32},
    Field{"score", FieldType::INT64}
});

Schema departmentsSchema({
    Field{"dept", FieldType::STRING32},
    Field{"building", FieldType::STRING32}
});

int main(int argc, char* argv[])
{
    std::filesystem::path directoryPath = directoryString;
    std::filesystem::remove_all(directoryString);
    std::shared_ptr<Database> database = std::make_shared<Database>(directoryPath);

    std::vector<std::pair<Tuple, RID>> studentRIDs;
    std::cout << "\t---Step 1 Test---";
    try{
        std::cout << "\t" << ((step_one(database, studentRIDs)) ? "[Pass" : "[Fail") << "]\n";
    }
    catch(std::exception& e)
    {
        std::cout << "[Fail]: " << e.what();
        return -1;
    }

    std::cout << "\t---Step 2 Test---";
    try{
        std::cout << "\t" << ((step_two(database, studentRIDs)) ? "[Pass" : "[Fail") << "]\n";
    }
    catch(std::exception& e)
    {
        std::cout << "[Fail]: " << e.what();
        return -1;
    }

    std::cout << "\t---Step 3 Test---";
    try{
        std::cout << "\t" << ((step_three(database, studentRIDs)) ? "[Pass" : "[Fail") << "]\n";
    }
    catch(std::exception& e)
    {
        std::cout << "[Fail]: " << e.what();
        return -1;
    }

    return 0;
    
}

/*
    Create Tables w/ Insert
    students(id INT64, name STRING32, dept STRING32, score INT64) 

    (1, "Alice", "CS",   90) 

    (2, "Bob",   "CS",   75) 

    (3, "Carol", "Math", 85) 

    (4, "Dan",   "Math", 75) 

    (5, "Eve",   "CS",   90) 
    Return Count = 5

    

    departments(dept STRING32, building STRING32) 

    ("CS",   "Tech") 

    ("Math", "Science") 

    ("Art",  "Studio") 
    Return Count = 3

    Sequential Scan must return all 5 students & all 3 departments
    Retain student RIDs
*/
bool step_one(std::shared_ptr<Database> database, std::vector<std::pair<Tuple, RID>>& studentRIDs)
{
    std::vector<Row> studentRows = 
    {
        Row{Tuple({int64_t{1}, std::string{"Alice"}, std::string{"CS"}, int64_t{90}})},
        Row{Tuple({int64_t{2}, std::string{"Bob"}, std::string{"CS"}, int64_t{75}})},
        Row{Tuple({int64_t{3}, std::string{"Carol"}, std::string{"Math"}, int64_t{85}})},
        Row{Tuple({int64_t{4}, std::string{"Dan"}, std::string{"Math"}, int64_t{75}})},
        Row{Tuple({int64_t{5}, std::string{"Eve"}, std::string{"CS"}, int64_t{90}})}
    };

    std::vector<Row> deptRows = 
    {
        Row{Tuple({std::string{"CS"}, std::string{"Tech"}})},
        Row{Tuple({std::string{"Math"}, std::string{"Science"}})},
        Row{Tuple({std::string{"Art"}, std::string{"Studio"}})}
    };

    std::shared_ptr<const std::vector<Row>> studentRowsPointer = std::make_shared<const std::vector<Row>>(std::move(studentRows));
    std::shared_ptr<const std::vector<Row>> deptRowsPointer = std::make_shared<const std::vector<Row>>(std::move(deptRows));

    database->createTable("students", studentSchema);
    database->createTable("depts", departmentsSchema);

    std::unique_ptr<Executor> studentValues = std::make_unique<ValuesExecutor>(studentRowsPointer, std::make_shared<const QuerySchema>(studentSchema));
    std::unique_ptr<Executor> deptValues = std::make_unique<ValuesExecutor>(deptRowsPointer, std::make_shared<const QuerySchema>(departmentsSchema));

    InsertExecutor studentInsert(database, "students", std::move(studentValues));
    InsertExecutor deptInsert(database, "depts", std::move(deptValues));

    studentInsert.Init();
    deptInsert.Init();

    Row result;

    // Insert Students
    studentInsert.Next(result);

    if(std::get<int64_t>(result.tuple.getTupleValueIndex(0)) != 5) return false;

    std::vector<std::pair<Tuple, RID>> scanResult = database->scan("students");
    if(scanResult.size() != 5) return false;
    studentRIDs = scanResult;

    // Insert Departments
    deptInsert.Next(result);
    if(std::get<int64_t>(result.tuple.getTupleValueIndex(0)) != 3) return false;

    scanResult = database->scan("depts");
    if(scanResult.size() != 3) return false;

    return true;
}

/*
    Filter & Project

    Execute the following:
    SELECT name, score + 5 AS adjusted_score 
    FROM students WHERE dept = 'CS' AND score >= 90; 

    Expect:  ("Alice", 95) and ("Eve", 95)
    Check filtering with score > 100 returns no rows
*/
bool step_two(std::shared_ptr<Database> database, std::vector<std::pair<Tuple, RID>>& studentRIDS)
{
    // Get rows
    std::unique_ptr<Executor> scan = std::make_unique<SequentialScanExecutor>(database, "students");

    // Make filter
    std::unique_ptr<Expression> filterExpression = 
        std::make_unique<AndExpression>(
            std::make_unique<GreaterThanEqualExpression>(
                std::make_unique<ColumnExpression>(3),
                std::make_unique<ConstantExpression>(int64_t{90})
            ),
            std::make_unique<EqualExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(std::string{"CS"})
            )
    );
    
    // Use Filter
    std::unique_ptr<Executor> filter = std::make_unique<FilterExecutor>(std::move(scan), std::move(filterExpression));

    // Select
    std::vector<ProjectionColumn> projectionList;

    projectionList.push_back(ProjectionColumn{Field{"name", FieldType::STRING32}, std::make_unique<ColumnExpression>(1)});

    projectionList.push_back(ProjectionColumn{Field{"adjusted_score", FieldType::INT64}, 
        std::make_unique<AdditionExpression>(
            std::make_unique<ColumnExpression>(3), 
            std::make_unique<ConstantExpression>(int64_t{5})
        )} // Score + 5, 
    );

    ProjectionExecutor projection(std::move(filter), std::move(projectionList));

    projection.Init();

    Row row;

    std::vector<Row> projectionRows;

    while(projection.Next(row))
    {
        projectionRows.push_back(row);
    }
    projection.Close();

    // Check Results
    if(projectionRows.size() != 2) return false;

    if(std::get<std::string>(projectionRows[0].tuple.getTupleValueIndex(0)) != "Alice") return false;
    if(std::get<int64_t>(projectionRows[0].tuple.getTupleValueIndex(1)) != int64_t{95}) return false;

    if(std::get<std::string>(projectionRows[1].tuple.getTupleValueIndex(0)) != "Eve") return false;
    if(std::get<int64_t>(projectionRows[1].tuple.getTupleValueIndex(1)) != int64_t{95}) return false;

    // Check filtering with score > 100 returns no results
    
    std::unique_ptr<Executor> scan2 = std::make_unique<SequentialScanExecutor>(database, "students");

    std::unique_ptr<Expression> filterExpression2 = 
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(3),
            std::make_unique<ConstantExpression>(int64_t{100})
        );

    std::unique_ptr<Executor> filter2 = std::make_unique<FilterExecutor>(std::move(scan2), std::move(filterExpression2));

    std::vector<ProjectionColumn> projectionList2;

    projectionList2.push_back(ProjectionColumn{Field{"name", FieldType::STRING32}, std::make_unique<ColumnExpression>(1)});

    projectionList2.push_back(ProjectionColumn{Field{"adjusted_score", FieldType::INT64}, 
        std::make_unique<AdditionExpression>(
            std::make_unique<ColumnExpression>(3), 
            std::make_unique<ConstantExpression>(int64_t{5})
        )} // Score + 5, 
    );

    ProjectionExecutor projection2(std::move(filter2), std::move(projectionList2));
    projection2.Init();

    Row row2;
    std::vector<Row> projectionRows2;
    
    while(projection2.Next(row))
    {
        projectionRows2.push_back(row);
    }
    projection2.Close();

    if(!projectionRows2.empty()) return false;

    return true;
}

/*
    Join

    Join students to departments on equal dept values
    Project name & building
    Expect:  ("Alice", "Tech"), ("Bob", "Tech"), ("Carol", "Science"), ("Dan", "Science"), and ("Eve", "Tech")
    Art contributes no rows!
*/
bool step_three(std::shared_ptr<Database> database, std::vector<std::pair<Tuple, RID>>& studentRIDS)
{
    std::unique_ptr<Executor> studentScan = std::make_unique<SequentialScanExecutor>(database, "students");
    std::unique_ptr<Executor> deptScan = std::make_unique<SequentialScanExecutor>(database, "depts");

    std::unique_ptr<Expression> filterExpression = 
        std::make_unique<EqualExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ColumnExpression>(4)
    );

    std::unique_ptr<Executor> join = std::make_unique<NestedLoopExecutor>(
        std::move(studentScan), std::move(deptScan), std::move(filterExpression)
    );

    std::vector<ProjectionColumn> projectionList;
    projectionList.push_back(ProjectionColumn{Field{"name", FieldType::STRING32}, std::make_unique<ColumnExpression>(1)});
    projectionList.push_back(ProjectionColumn{Field{"building", FieldType::STRING32}, std::make_unique<ColumnExpression>(5)});

    ProjectionExecutor projection(std::move(join), std::move(projectionList));
    projection.Init();
    Row row;

    std::vector<Row> projectionRows;

    while(projection.Next(row))
    {
        projectionRows.push_back(row);
    }
    projection.Close();

    if(projectionRows.size() != 5) return false;

    if(std::get<std::string>(projectionRows[0].tuple.getTupleValueIndex(0)) != "Alice") return false;
    if(std::get<std::string>(projectionRows[0].tuple.getTupleValueIndex(1)) != "Tech") return false;

    if(std::get<std::string>(projectionRows[1].tuple.getTupleValueIndex(0)) != "Bob") return false;
    if(std::get<std::string>(projectionRows[1].tuple.getTupleValueIndex(1)) != "Tech") return false;

    if(std::get<std::string>(projectionRows[2].tuple.getTupleValueIndex(0)) != "Carol") return false;
    if(std::get<std::string>(projectionRows[2].tuple.getTupleValueIndex(1)) != "Science") return false;

    if(std::get<std::string>(projectionRows[3].tuple.getTupleValueIndex(0)) != "Dan") return false;
    if(std::get<std::string>(projectionRows[3].tuple.getTupleValueIndex(1)) != "Science") return false;

    if(std::get<std::string>(projectionRows[4].tuple.getTupleValueIndex(0)) != "Eve") return false;
    if(std::get<std::string>(projectionRows[4].tuple.getTupleValueIndex(1)) != "Tech") return false;

    return true;
}