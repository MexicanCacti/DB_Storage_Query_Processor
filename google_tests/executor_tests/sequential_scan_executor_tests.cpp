
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <variant>

#include "sequential_scan_executor.hpp"
#include "database.hpp"
#include "schema.hpp"
#include "tuple.hpp"

namespace fs = std::filesystem;

// ============================================================
// Test Fixture
// ============================================================

class SequentialScanExecutorTest : public ::testing::Test
{
protected:
    fs::path databaseDirectory;
    std::shared_ptr<Database> database;

    std::vector<RID> insertedRIDs;

    void SetUp() override
    {
        databaseDirectory =
            fs::temp_directory_path() /
            ("minidb_scan_" +
             std::to_string(
                 reinterpret_cast<std::uintptr_t>(this)
             ));

        fs::remove_all(databaseDirectory);
        fs::create_directories(databaseDirectory);

        database = std::make_shared<Database>(
            databaseDirectory
        );

        // Database still uses the storage Schema.
        Schema schema({
            Field{"id", FieldType::INT64},
            Field{"name", FieldType::STRING32},
            Field{"score", FieldType::INT64}
        });

        database->createTable("students", schema);
    }

    void TearDown() override
    {
        database.reset();
        fs::remove_all(databaseDirectory);
    }

    void InsertStudents()
    {
        insertedRIDs.push_back(
            database->insertTuple(
                "students",
                Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    int64_t{90}
                })
            )
        );

        insertedRIDs.push_back(
            database->insertTuple(
                "students",
                Tuple({
                    int64_t{2},
                    std::string{"Bob"},
                    int64_t{75}
                })
            )
        );

        insertedRIDs.push_back(
            database->insertTuple(
                "students",
                Tuple({
                    int64_t{3},
                    std::string{"Carol"},
                    int64_t{85}
                })
            )
        );
    }

    static void ExpectRIDEqual(
        const RID& actual,
        const RID& expected
    )
    {
        EXPECT_TRUE(actual.pageID == expected.pageID);
        EXPECT_EQ(actual.tupleSlot, expected.tupleSlot);
    }

    static int64_t GetStudentID(const Row& row)
    {
        return std::get<int64_t>(
            row.tuple.getTupleValues().at(0)
        );
    }

    static void ExpectStudent(
        const Row& row,
        int64_t expectedID,
        const std::string& expectedName,
        int64_t expectedScore
    )
    {
        const auto& values = row.tuple.getTupleValues();

        ASSERT_EQ(values.size(), 3);

        ASSERT_TRUE(
            std::holds_alternative<int64_t>(values[0])
        );

        ASSERT_TRUE(
            std::holds_alternative<std::string>(values[1])
        );

        ASSERT_TRUE(
            std::holds_alternative<int64_t>(values[2])
        );

        EXPECT_EQ(
            std::get<int64_t>(values[0]),
            expectedID
        );

        EXPECT_EQ(
            std::get<std::string>(values[1]),
            expectedName
        );

        EXPECT_EQ(
            std::get<int64_t>(values[2]),
            expectedScore
        );
    }

    static std::vector<Row> CollectRows(
        Executor& executor
    )
    {
        std::vector<Row> results;
        Row row;

        while (executor.Next(row))
        {
            results.push_back(row);
        }

        return results;
    }

    static void ExpectQuerySchema(
        const QuerySchema& schema
    )
    {
        const auto& fields = schema.getFieldList();

        ASSERT_EQ(fields.size(), 3);

        EXPECT_EQ(fields[0].name, "id");
        EXPECT_EQ(fields[0].type, FieldType::INT64);

        EXPECT_EQ(fields[1].name, "name");
        EXPECT_EQ(fields[1].type, FieldType::STRING32);

        EXPECT_EQ(fields[2].name, "score");
        EXPECT_EQ(fields[2].type, FieldType::INT64);
    }
};

// ============================================================
// 1. Empty table produces no rows
// ============================================================

TEST_F(SequentialScanExecutorTest, EmptyTable)
{
    SequentialScanExecutor executor(database, "students");

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 2. Scan returns all live tuples exactly once
// ============================================================

TEST_F(SequentialScanExecutorTest, ReturnsAllTuples)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    // Do not assume scan order.
    std::sort(
        results.begin(),
        results.end(),
        [](const Row& a, const Row& b)
        {
            return GetStudentID(a) < GetStudentID(b);
        }
    );

    ExpectStudent(results[0], 1, "Alice", 90);
    ExpectStudent(results[1], 2, "Bob", 75);
    ExpectStudent(results[2], 3, "Carol", 85);

    executor.Close();
}

// ============================================================
// 3. OutputSchema returns QuerySchema before Init
// ============================================================

TEST_F(SequentialScanExecutorTest, OutputSchemaBeforeInit)
{
    SequentialScanExecutor executor(database, "students");

    const QuerySchema& querySchema =
        executor.OutputSchema();

    ExpectQuerySchema(querySchema);
}

// ============================================================
// 4. Storage Schema is converted to QuerySchema correctly
// ============================================================

TEST_F(SequentialScanExecutorTest, ConvertsStorageSchema)
{
    SequentialScanExecutor executor(database, "students");

    std::string tableName = "students";

    TableHeader tableInfo =
        database->queryTableInfo(tableName);

    const Schema& storageSchema =
        tableInfo.getSchema();

    const QuerySchema& querySchema =
        executor.OutputSchema();

    const auto& storageFields =
        storageSchema.getFieldList();

    const auto& queryFields =
        querySchema.getFieldList();

    ASSERT_EQ(storageFields.size(), queryFields.size());

    for (std::size_t i = 0; i < storageFields.size(); ++i)
    {
        EXPECT_EQ(
            storageFields[i].name,
            queryFields[i].name
        );

        EXPECT_EQ(
            storageFields[i].type,
            queryFields[i].type
        );
    }
}

// ============================================================
// 5. Source references preserve table identity and RID
// ============================================================

TEST_F(SequentialScanExecutorTest, PreservesSourceReferences)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    Row row;
    std::size_t count = 0;

    while (executor.Next(row))
    {
        ASSERT_TRUE(row.source.has_value());

        EXPECT_EQ(
            row.source->tableName,
            "students"
        );

        int64_t id = GetStudentID(row);

        ASSERT_GE(id, 1);
        ASSERT_LE(id, 3);

        const RID& expectedRID =
            insertedRIDs[
                static_cast<std::size_t>(id - 1)
            ];

        ExpectRIDEqual(
            row.source->rid,
            expectedRID
        );

        Tuple storedTuple = database->readTuple(
            "students",
            row.source->rid
        );

        EXPECT_EQ(
            storedTuple.getTupleValues(),
            row.tuple.getTupleValues()
        );

        ++count;
    }

    EXPECT_EQ(count, 3);

    executor.Close();
}

// ============================================================
// 6. Logically deleted tuples are skipped
// ============================================================

TEST_F(SequentialScanExecutorTest, SkipsDeletedTuples)
{
    InsertStudents();

    database->deleteTuple(
        "students",
        insertedRIDs[1]
    );

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 2);

    std::sort(
        results.begin(),
        results.end(),
        [](const Row& a, const Row& b)
        {
            return GetStudentID(a) < GetStudentID(b);
        }
    );

    ExpectStudent(results[0], 1, "Alice", 90);
    ExpectStudent(results[1], 3, "Carol", 85);

    executor.Close();
}

// ============================================================
// 7. Exhaustion remains exhausted
// ============================================================

TEST_F(SequentialScanExecutorTest, ExhaustionReturnsFalse)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    Row row;

    for (int i = 0; i < 3; ++i)
    {
        ASSERT_TRUE(executor.Next(row));
    }

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 8. Init resets the scan
// ============================================================

TEST_F(SequentialScanExecutorTest, InitResetsIteration)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    auto firstResults = CollectRows(executor);

    executor.Close();
    executor.Init();

    auto secondResults = CollectRows(executor);

    ASSERT_EQ(firstResults.size(), 3);
    ASSERT_EQ(secondResults.size(), 3);

    for (std::size_t i = 0; i < firstResults.size(); ++i)
    {
        EXPECT_EQ(
            firstResults[i].tuple.getTupleValues(),
            secondResults[i].tuple.getTupleValues()
        );

        ASSERT_TRUE(firstResults[i].source.has_value());
        ASSERT_TRUE(secondResults[i].source.has_value());

        ExpectRIDEqual(
            firstResults[i].source->rid,
            secondResults[i].source->rid
        );
    }

    executor.Close();
}

// ============================================================
// 9. Close before exhaustion
// ============================================================

TEST_F(SequentialScanExecutorTest, CloseBeforeExhaustion)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));

    executor.Close();

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 3);

    executor.Close();
}

// ============================================================
// 10. Close can be called multiple times
// ============================================================

TEST_F(SequentialScanExecutorTest, CloseIsIdempotent)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    // Close before Init.
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 3);

    executor.Close();
}

// ============================================================
// 11. OutputSchema remains valid after Close
// ============================================================

TEST_F(SequentialScanExecutorTest, SchemaAvailableAfterClose)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    Row row;
    ASSERT_TRUE(executor.Next(row));

    executor.Close();

    ExpectQuerySchema(executor.OutputSchema());
}

// ============================================================
// 12. Returned rows remain valid after Next and Close
// ============================================================

TEST_F(SequentialScanExecutorTest, ReturnedRowsRemainValid)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    Row firstRow;
    Row secondRow;

    ASSERT_TRUE(executor.Next(firstRow));

    const auto savedValues =
        firstRow.tuple.getTupleValues();

    ASSERT_TRUE(firstRow.source.has_value());

    const RID savedRID = firstRow.source->rid;

    ASSERT_TRUE(executor.Next(secondRow));

    EXPECT_EQ(
        firstRow.tuple.getTupleValues(),
        savedValues
    );

    executor.Close();

    EXPECT_EQ(
        firstRow.tuple.getTupleValues(),
        savedValues
    );

    ASSERT_TRUE(firstRow.source.has_value());

    ExpectRIDEqual(
        firstRow.source->rid,
        savedRID
    );
}

// ============================================================
// 13. Reinitialization detects newly inserted tuples
// ============================================================

TEST_F(SequentialScanExecutorTest, ReinitSeesNewTuples)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    auto firstResults = CollectRows(executor);

    EXPECT_EQ(firstResults.size(), 3);

    executor.Close();

    database->insertTuple(
        "students",
        Tuple({
            int64_t{4},
            std::string{"Dan"},
            int64_t{75}
        })
    );

    executor.Init();

    auto secondResults = CollectRows(executor);

    ASSERT_EQ(secondResults.size(), 4);

    bool foundDan = false;

    for (const Row& row : secondResults)
    {
        if (GetStudentID(row) == 4)
        {
            ExpectStudent(row, 4, "Dan", 75);
            foundDan = true;
        }
    }

    EXPECT_TRUE(foundDan);

    executor.Close();
}

// ============================================================
// 14. Two scan executors operate independently
// ============================================================

TEST_F(SequentialScanExecutorTest, IndependentExecutors)
{
    InsertStudents();

    SequentialScanExecutor executor1(database, "students");
    SequentialScanExecutor executor2(database, "students");

    executor1.Init();
    executor2.Init();

    Row row1;

    ASSERT_TRUE(executor1.Next(row1));
    ASSERT_TRUE(executor1.Next(row1));

    // Executor2 should still return all three tuples.
    auto results = CollectRows(executor2);

    ASSERT_EQ(results.size(), 3);

    executor1.Close();
    executor2.Close();
}

// ============================================================
// 15. Closing one executor does not affect another
// ============================================================

TEST_F(SequentialScanExecutorTest, ClosingOneDoesNotAffectAnother)
{
    InsertStudents();

    SequentialScanExecutor executor1(database, "students");
    SequentialScanExecutor executor2(database, "students");

    executor1.Init();
    executor2.Init();

    executor1.Close();

    auto results = CollectRows(executor2);

    ASSERT_EQ(results.size(), 3);

    executor2.Close();
}

// ============================================================
// 16. Next after Close returns false
// ============================================================

TEST_F(SequentialScanExecutorTest, NextAfterClose)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();
    executor.Close();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));
}
