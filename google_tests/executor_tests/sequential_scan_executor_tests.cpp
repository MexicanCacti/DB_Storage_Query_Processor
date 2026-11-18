
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

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

    // Compare RID without requiring RID::operator==
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
        int64_t id,
        const std::string& name,
        int64_t score
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

        EXPECT_EQ(std::get<int64_t>(values[0]), id);
        EXPECT_EQ(std::get<std::string>(values[1]), name);
        EXPECT_EQ(std::get<int64_t>(values[2]), score);
    }

    static std::vector<Row> CollectRows(
        SequentialScanExecutor& executor
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
};

// ============================================================
// Test 1: Scan an empty table
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
// Test 2: Scan returns all inserted tuples exactly once
// ============================================================

TEST_F(SequentialScanExecutorTest, ReturnsAllTuples)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

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
// Test 3: OutputSchema matches the stored table
// ============================================================

TEST_F(SequentialScanExecutorTest, OutputSchemaMatchesTable)
{
    SequentialScanExecutor executor(database, "students");

    // Schema must be available before Init().
    const Schema& schema = executor.OutputSchema();

    const auto& fields = schema.getFieldList();

    ASSERT_EQ(fields.size(), 3);

    EXPECT_EQ(fields[0].name, "id");
    EXPECT_EQ(fields[0].type, FieldType::INT64);

    EXPECT_EQ(fields[1].name, "name");
    EXPECT_EQ(fields[1].type, FieldType::STRING32);

    EXPECT_EQ(fields[2].name, "score");
    EXPECT_EQ(fields[2].type, FieldType::INT64);
}

// ============================================================
// Test 4: Every scanned row has the correct source reference
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
            insertedRIDs[static_cast<std::size_t>(id - 1)];

        ExpectRIDEqual(
            row.source->rid,
            expectedRID
        );

        // Verify the source RID can retrieve the same tuple.
        Tuple stored = database->readTuple(
            "students",
            row.source->rid
        );

        EXPECT_EQ(
            stored.getTupleValues(),
            row.tuple.getTupleValues()
        );

        ++count;
    }

    EXPECT_EQ(count, 3);

    executor.Close();
}

// ============================================================
// Test 5: Deleted tuples are excluded
// ============================================================

TEST_F(SequentialScanExecutorTest, SkipsDeletedTuples)
{
    InsertStudents();

    // Delete Bob.
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
// Test 6: Next remains false after exhaustion
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
// Test 7: Init resets the scan
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

    // Compare corresponding rows.
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
// Test 8: Close before exhaustion allows reuse
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
// Test 9: Close is idempotent
// ============================================================

TEST_F(SequentialScanExecutorTest, CloseIsIdempotent)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    // Close before initialization.
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    // Must still support a new execution.
    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 3);

    executor.Close();
}

// ============================================================
// Test 10: Returned rows remain valid after Close
// ============================================================

TEST_F(SequentialScanExecutorTest, ReturnedRowsRemainValid)
{
    InsertStudents();

    SequentialScanExecutor executor(database, "students");

    executor.Init();

    Row firstRow;
    Row secondRow;

    ASSERT_TRUE(executor.Next(firstRow));

    // Save the first row's values before advancing.
    const auto savedValues =
        firstRow.tuple.getTupleValues();

    ASSERT_TRUE(executor.Next(secondRow));

    // Advancing must not change the first row.
    EXPECT_EQ(
        firstRow.tuple.getTupleValues(),
        savedValues
    );

    executor.Close();

    // Closing must not invalidate either returned row.
    EXPECT_EQ(
        firstRow.tuple.getTupleValues(),
        savedValues
    );

    ASSERT_TRUE(firstRow.source.has_value());
    ASSERT_TRUE(secondRow.source.has_value());

    Tuple stored = database->readTuple(
        "students",
        firstRow.source->rid
    );

    EXPECT_EQ(
        stored.getTupleValues(),
        savedValues
    );
}

// ============================================================
// Test 11: Reinitialization sees newly inserted tuples
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

    for (const auto& row : secondResults)
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
// Test 12: Two executors scan independently
// ============================================================

TEST_F(SequentialScanExecutorTest, IndependentExecutors)
{
    InsertStudents();

    SequentialScanExecutor executor1(database, "students");
    SequentialScanExecutor executor2(database, "students");

    executor1.Init();
    executor2.Init();

    Row row1;
    Row row2;

    ASSERT_TRUE(executor1.Next(row1));

    // Advance executor1 again.
    ASSERT_TRUE(executor1.Next(row1));

    // Executor2 must still have all three rows available.
    auto results = CollectRows(executor2);

    EXPECT_EQ(results.size(), 3);

    executor1.Close();
    executor2.Close();
}

TEST_F(SequentialScanExecutorTest, DatabaseSupportsRepeatedScans)
{
    InsertStudents();

    std::vector<std::pair<Tuple, RID>> firstScan;
    std::vector<std::pair<Tuple, RID>> secondScan;

    ASSERT_NO_THROW({
        firstScan = database->scan("students");
    });

    ASSERT_NO_THROW({
        secondScan = database->scan("students");
    });

    EXPECT_EQ(firstScan.size(), 3);
    EXPECT_EQ(secondScan.size(), 3);
}

TEST_F(SequentialScanExecutorTest, MultipleExecutorConstruction)
{
    InsertStudents();

    ASSERT_NO_THROW({
        SequentialScanExecutor executor1(database, "students");
        SequentialScanExecutor executor2(database, "students");
    });
}