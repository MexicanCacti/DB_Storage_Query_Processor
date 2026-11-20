
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <variant>

#include "insert_executor.hpp"
#include "values_executor.hpp"
#include "database.hpp"
#include "schema.hpp"
#include "tuple.hpp"
#include "types.hpp"

namespace fs = std::filesystem;

// ============================================================
// Test Fixture
// ============================================================

class InsertExecutorTest : public ::testing::Test
{
protected:
    fs::path databaseDirectory;

    std::shared_ptr<Database> database;

    std::shared_ptr<const QuerySchema> querySchema;

    void SetUp() override
    {
        databaseDirectory =
            fs::temp_directory_path() /
            ("minidb_insert_" +
             std::to_string(
                 reinterpret_cast<std::uintptr_t>(this)
             ));

        fs::remove_all(databaseDirectory);
        fs::create_directories(databaseDirectory);

        database = std::make_shared<Database>(
            databaseDirectory
        );

        Schema storageSchema({
            Field{"id", FieldType::INT64},
            Field{"name", FieldType::STRING32},
            Field{"score", FieldType::INT64}
        });

        database->createTable("students", storageSchema);

        querySchema = std::make_shared<const QuerySchema>(
            storageSchema
        );
    }

    void TearDown() override
    {
        database.reset();
        fs::remove_all(databaseDirectory);
    }

    std::unique_ptr<Executor> MakeValuesExecutor(
        std::vector<Row> rows
    )
    {
        auto rowPointer =
            std::make_shared<const std::vector<Row>>(
                std::move(rows)
            );

        return std::make_unique<ValuesExecutor>(
            rowPointer,
            querySchema
        );
    }

    std::unique_ptr<Executor> MakeStudentValues()
    {
        return MakeValuesExecutor({
            Row{Tuple({
                int64_t{1},
                std::string{"Alice"},
                int64_t{90}
            })},
            Row{Tuple({
                int64_t{2},
                std::string{"Bob"},
                int64_t{75}
            })},
            Row{Tuple({
                int64_t{3},
                std::string{"Carol"},
                int64_t{85}
            })}
        });
    }

    static int64_t GetAffectedRows(const Row& row)
    {
        const auto& values = row.tuple.getTupleValues();

        EXPECT_EQ(values.size(), 1);

        if (values.size() != 1)
            return -1;

        EXPECT_TRUE(
            std::holds_alternative<int64_t>(values[0])
        );

        if (!std::holds_alternative<int64_t>(values[0]))
            return -1;

        return std::get<int64_t>(values[0]);
    }

    std::vector<std::pair<Tuple, RID>> ScanStudents()
    {
        return database->scan("students");
    }

    static int64_t GetStudentID(const Tuple& tuple)
    {
        return std::get<int64_t>(
            tuple.getTupleValues().at(0)
        );
    }
};

// ============================================================
// 1. Insert multiple rows
// ============================================================

TEST_F(InsertExecutorTest, InsertsMultipleRows)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 3);

    auto storedRows = ScanStudents();

    ASSERT_EQ(storedRows.size(), 3);

    executor.Close();
}

// ============================================================
// 2. Inserted values match original input
// ============================================================

TEST_F(InsertExecutorTest, InsertedValuesAreCorrect)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    auto storedRows = ScanStudents();

    ASSERT_EQ(storedRows.size(), 3);

    std::vector<int64_t> ids;

    for (const auto& [tuple, rid] : storedRows)
    {
        ids.push_back(GetStudentID(tuple));

        const auto& values = tuple.getTupleValues();

        ASSERT_EQ(values.size(), 3);

        int64_t id = std::get<int64_t>(values[0]);

        if (id == 1)
        {
            EXPECT_EQ(
                std::get<std::string>(values[1]),
                "Alice"
            );

            EXPECT_EQ(
                std::get<int64_t>(values[2]),
                90
            );
        }
        else if (id == 2)
        {
            EXPECT_EQ(
                std::get<std::string>(values[1]),
                "Bob"
            );

            EXPECT_EQ(
                std::get<int64_t>(values[2]),
                75
            );
        }
        else if (id == 3)
        {
            EXPECT_EQ(
                std::get<std::string>(values[1]),
                "Carol"
            );

            EXPECT_EQ(
                std::get<int64_t>(values[2]),
                85
            );
        }
        else
        {
            FAIL() << "Unexpected student ID: " << id;
        }
    }

    std::sort(ids.begin(), ids.end());

    EXPECT_EQ(
        ids,
        (std::vector<int64_t>{1, 2, 3})
    );

    executor.Close();
}

// ============================================================
// 3. Insert returns exactly one result row
// ============================================================

TEST_F(InsertExecutorTest, ReturnsExactlyOneCountRow)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 3);

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ============================================================
// 4. Repeated Next does not repeat inserts
// ============================================================

TEST_F(InsertExecutorTest, DoesNotRepeatWrites)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 3);

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    auto storedRows = ScanStudents();

    EXPECT_EQ(storedRows.size(), 3);

    executor.Close();
}

// ============================================================
// 5. Init does not insert any rows
// ============================================================

TEST_F(InsertExecutorTest, InitDoesNotModifyStorage)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    EXPECT_TRUE(ScanStudents().empty());

    executor.Close();

    EXPECT_TRUE(ScanStudents().empty());
}

// ============================================================
// 6. Close before Next performs no writes
// ============================================================

TEST_F(InsertExecutorTest, CloseBeforeNext)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();
    executor.Close();

    EXPECT_TRUE(ScanStudents().empty());
}

// ============================================================
// 7. Empty input returns affected_rows = 0
// ============================================================

TEST_F(InsertExecutorTest, EmptyInput)
{
    InsertExecutor executor(
        database,
        "students",
        MakeValuesExecutor({})
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 0);

    EXPECT_FALSE(executor.Next(result));

    EXPECT_TRUE(ScanStudents().empty());

    executor.Close();
}

// ============================================================
// 8. Output schema is affected_rows INT64
// ============================================================

TEST_F(InsertExecutorTest, OutputSchemaIsAffectedRows)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    const QuerySchema& schema =
        executor.OutputSchema();

    const auto& fields = schema.getFieldList();

    ASSERT_EQ(fields.size(), 1);

    EXPECT_EQ(fields[0].name, "Affected Rows");
    EXPECT_EQ(fields[0].type, FieldType::INT64);
}

// ============================================================
// 9. Reinitialization performs a new insertion
// ============================================================

TEST_F(InsertExecutorTest, ReinitPerformsNewInsertion)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 3);

    executor.Close();

    EXPECT_EQ(ScanStudents().size(), 3);

    // Reinitialization is a new modification.
    executor.Init();

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 3);

    EXPECT_EQ(ScanStudents().size(), 6);

    executor.Close();
}

// ============================================================
// 10. Close is idempotent
// ============================================================

TEST_F(InsertExecutorTest, CloseIsIdempotent)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    EXPECT_EQ(ScanStudents().size(), 3);
}

// ============================================================
// 11. Returned count row remains valid
// ============================================================

TEST_F(InsertExecutorTest, ReturnedRowRemainsValid)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    Row savedResult = result;

    EXPECT_FALSE(executor.Next(result));

    executor.Close();

    EXPECT_EQ(GetAffectedRows(savedResult), 3);

    EXPECT_FALSE(savedResult.source.has_value());
}

// ============================================================
// 12. Reject incorrect column count
// ============================================================

TEST_F(InsertExecutorTest, RejectsWrongColumnCount)
{
    auto invalidRows = MakeValuesExecutor({
        Row{Tuple({
            int64_t{1},
            std::string{"Alice"}
        })}
    });

    InsertExecutor executor(
        database,
        "students",
        std::move(invalidRows)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::exception
    );

    EXPECT_TRUE(ScanStudents().empty());

    executor.Close();
}

// ============================================================
// 13. Reject incompatible field types
// ============================================================

TEST_F(InsertExecutorTest, RejectsWrongFieldType)
{
    auto invalidRows = MakeValuesExecutor({
        Row{Tuple({
            std::string{"NotAnInteger"},
            std::string{"Alice"},
            int64_t{90}
        })}
    });

    InsertExecutor executor(
        database,
        "students",
        std::move(invalidRows)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::exception
    );

    EXPECT_TRUE(ScanStudents().empty());

    executor.Close();
}

// ============================================================
// 14. Reject strings exceeding STRING32
// ============================================================

TEST_F(InsertExecutorTest, RejectsOversizedString)
{
    std::string longName(33, 'A');

    auto invalidRows = MakeValuesExecutor({
        Row{Tuple({
            int64_t{1},
            longName,
            int64_t{90}
        })}
    });

    InsertExecutor executor(
        database,
        "students",
        std::move(invalidRows)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::exception
    );

    EXPECT_TRUE(ScanStudents().empty());

    executor.Close();
}

// ============================================================
// 15. Insert into nonexistent table
// ============================================================

TEST_F(InsertExecutorTest, RejectsNonexistentTable)
{
    InsertExecutor executor(
        database,
        "nonexistent",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::exception
    );

    executor.Close();
}

// ============================================================
// 16. Inserted records remain readable by RID
// ============================================================

TEST_F(InsertExecutorTest, InsertedRowsReadableByRID)
{
    InsertExecutor executor(
        database,
        "students",
        MakeStudentValues()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    auto storedRows = ScanStudents();

    ASSERT_EQ(storedRows.size(), 3);

    for (const auto& [tuple, rid] : storedRows)
    {
        Tuple retrieved =
            database->readTuple("students", rid);

        EXPECT_EQ(
            retrieved.getTupleValues(),
            tuple.getTupleValues()
        );
    }

    executor.Close();
}

// ============================================================
// 17. Multiple insert executors share the database
// ============================================================

TEST_F(InsertExecutorTest, MultipleInsertExecutors)
{
    InsertExecutor executor1(
        database,
        "students",
        MakeValuesExecutor({
            Row{Tuple({
                int64_t{1},
                std::string{"Alice"},
                int64_t{90}
            })}
        })
    );

    InsertExecutor executor2(
        database,
        "students",
        MakeValuesExecutor({
            Row{Tuple({
                int64_t{2},
                std::string{"Bob"},
                int64_t{75}
            })}
        })
    );

    executor1.Init();
    executor2.Init();

    Row result;

    ASSERT_TRUE(executor1.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 1);

    ASSERT_TRUE(executor2.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 1);

    EXPECT_EQ(ScanStudents().size(), 2);

    executor1.Close();
    executor2.Close();
}


TEST_F(InsertExecutorTest, InsertedRowsPersistAfterReopen)
{
    {
        InsertExecutor executor(
            database,
            "students",
            MakeStudentValues()
        );

        executor.Init();

        Row result;

        ASSERT_TRUE(executor.Next(result));
        EXPECT_EQ(GetAffectedRows(result), 3);

        executor.Close();
    }

    database->flushAllPages();
    database.reset();

    // Reopen the existing database directory.
    database = std::make_shared<Database>(
        databaseDirectory
    );

    auto storedRows = database->scan("students");

    ASSERT_EQ(storedRows.size(), 3);

    std::vector<int64_t> ids;

    for (const auto& [tuple, rid] : storedRows)
    {
        ids.push_back(GetStudentID(tuple));
    }

    std::sort(ids.begin(), ids.end());

    EXPECT_EQ(
        ids,
        (std::vector<int64_t>{1, 2, 3})
    );
}
