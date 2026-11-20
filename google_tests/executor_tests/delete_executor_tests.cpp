
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "delete_executor.hpp"
#include "sequential_scan_executor.hpp"
#include "filter_executor.hpp"
#include "values_executor.hpp"

#include "equal_expression.hpp"
#include "greater_than_expression.hpp"
#include "constant_expression.hpp"
#include "column_expression.hpp"

namespace fs = std::filesystem;

// ============================================================
// Test Fixture
// ============================================================

class DeleteExecutorTest : public ::testing::Test
{
protected:
    fs::path databaseDirectory;
    std::shared_ptr<Database> database;

    std::vector<RID> insertedRIDs;

    void SetUp() override
    {
        databaseDirectory =
            fs::temp_directory_path() /
            ("minidb_delete_" +
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

        InsertStudents();
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

        insertedRIDs.push_back(
            database->insertTuple(
                "students",
                Tuple({
                    int64_t{4},
                    std::string{"Dan"},
                    int64_t{75}
                })
            )
        );

        insertedRIDs.push_back(
            database->insertTuple(
                "students",
                Tuple({
                    int64_t{5},
                    std::string{"Eve"},
                    int64_t{90}
                })
            )
        );
    }

    std::unique_ptr<Executor> MakeScan()
    {
        return std::make_unique<SequentialScanExecutor>(
            database,
            "students"
        );
    }

    std::unique_ptr<Executor> MakeFilterByID(int64_t id)
    {
        return std::make_unique<FilterExecutor>(
            MakeScan(),
            std::make_unique<EqualExpression>(
                std::make_unique<ColumnExpression>(0),
                std::make_unique<ConstantExpression>(id)
            )
        );
    }

    std::unique_ptr<Executor> MakeFilterByScore(
        int64_t score
    )
    {
        return std::make_unique<FilterExecutor>(
            MakeScan(),
            std::make_unique<EqualExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(score)
            )
        );
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

    std::vector<int64_t> GetRemainingIDs()
    {
        std::vector<int64_t> ids;

        auto results = database->scan("students");

        for (const auto& [tuple, rid] : results)
        {
            ids.push_back(
                std::get<int64_t>(
                    tuple.getTupleValues().at(0)
                )
            );
        }

        std::sort(ids.begin(), ids.end());

        return ids;
    }

    static void ExpectRIDEqual(
        const RID& actual,
        const RID& expected
    )
    {
        EXPECT_TRUE(actual.pageID == expected.pageID);
        EXPECT_EQ(actual.tupleSlot, expected.tupleSlot);
    }
};

// ============================================================
// 1. Delete one matching row
// DELETE FROM students WHERE id = 2
// ============================================================

TEST_F(DeleteExecutorTest, DeletesSingleRow)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 1);

    EXPECT_EQ(
        GetRemainingIDs(),
        (std::vector<int64_t>{1, 3, 4, 5})
    );

    executor.Close();
}

// ============================================================
// 2. Delete multiple matching rows
// DELETE FROM students WHERE score = 75
// ============================================================

TEST_F(DeleteExecutorTest, DeletesMultipleRows)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByScore(75)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 2);

    EXPECT_EQ(
        GetRemainingIDs(),
        (std::vector<int64_t>{1, 3, 5})
    );

    executor.Close();
}

// ============================================================
// 3. Delete all rows using sequential scan
// ============================================================

TEST_F(DeleteExecutorTest, DeletesAllRows)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeScan()
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 5);

    EXPECT_TRUE(GetRemainingIDs().empty());

    executor.Close();
}

// ============================================================
// 4. No matching rows returns affected_rows = 0
// ============================================================

TEST_F(DeleteExecutorTest, NoMatchingRows)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(100)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 0);

    EXPECT_FALSE(executor.Next(result));

    EXPECT_EQ(
        GetRemainingIDs(),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );

    executor.Close();
}

// ============================================================
// 5. Delete returns exactly one count row
// ============================================================

TEST_F(DeleteExecutorTest, ReturnsExactlyOneCountRow)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 1);

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ============================================================
// 6. Repeated Next does not repeat deletion
// ============================================================

TEST_F(DeleteExecutorTest, DoesNotRepeatDeletes)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 1);

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    EXPECT_EQ(GetRemainingIDs().size(), 4);

    executor.Close();
}

// ============================================================
// 7. Init does not modify storage
// ============================================================

TEST_F(DeleteExecutorTest, InitDoesNotDeleteRows)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    EXPECT_EQ(
        GetRemainingIDs(),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );

    executor.Close();

    EXPECT_EQ(
        GetRemainingIDs(),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );
}

// ============================================================
// 8. Close before Next performs no deletion
// ============================================================

TEST_F(DeleteExecutorTest, CloseBeforeNext)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();
    executor.Close();

    EXPECT_EQ(
        GetRemainingIDs(),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );
}

// ============================================================
// 9. Deleted RID is no longer readable
// ============================================================

TEST_F(DeleteExecutorTest, DeletedRIDNotReadable)
{
    const RID bobRID = insertedRIDs[1];

    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 1);

    Tuple readTuple = database->readTuple("students", bobRID);
    EXPECT_TRUE( readTuple.getTupleValues().empty() );

    executor.Close();
}

// ============================================================
// 10. Other RIDs remain valid
// ============================================================

TEST_F(DeleteExecutorTest, OtherRIDsRemainValid)
{
    const RID aliceRID = insertedRIDs[0];
    const RID carolRID = insertedRIDs[2];

    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    Tuple alice = database->readTuple(
        "students",
        aliceRID
    );

    Tuple carol = database->readTuple(
        "students",
        carolRID
    );

    EXPECT_EQ(
        std::get<int64_t>(
            alice.getTupleValues().at(0)
        ),
        1
    );

    EXPECT_EQ(
        std::get<int64_t>(
            carol.getTupleValues().at(0)
        ),
        3
    );

    executor.Close();
}

// ============================================================
// 11. Output schema is affected_rows INT64
// ============================================================

TEST_F(DeleteExecutorTest, OutputSchemaIsAffectedRows)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    const auto& fields =
        executor.OutputSchema().getFieldList();

    ASSERT_EQ(fields.size(), 1);

    EXPECT_EQ(fields[0].name, "Affected Rows");
    EXPECT_EQ(fields[0].type, FieldType::INT64);
}

// ============================================================
// 12. Returned count row has no source reference
// ============================================================

TEST_F(DeleteExecutorTest, CountRowHasNoSourceReference)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_FALSE(result.source.has_value());

    executor.Close();
}

// ============================================================
// 13. Close is idempotent
// ============================================================

TEST_F(DeleteExecutorTest, CloseIsIdempotent)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    EXPECT_EQ(GetRemainingIDs().size(), 4);
}

// ============================================================
// 14. Reinitialization is a new modification
// ============================================================

TEST_F(DeleteExecutorTest, ReinitPerformsNewDeletion)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByScore(75)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 2);

    executor.Close();

    EXPECT_EQ(GetRemainingIDs().size(), 3);

    // Insert another qualifying student.
    database->insertTuple(
        "students",
        Tuple({
            int64_t{6},
            std::string{"Frank"},
            int64_t{75}
        })
    );

    executor.Init();

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 1);

    EXPECT_EQ(
        GetRemainingIDs(),
        (std::vector<int64_t>{1, 3, 5})
    );

    executor.Close();
}

// ============================================================
// 15. Returned count remains valid after Close
// ============================================================

TEST_F(DeleteExecutorTest, ReturnedCountRemainsValid)
{
    DeleteExecutor executor(
        database,
        "students",
        MakeFilterByID(2)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    Row savedResult = result;

    EXPECT_FALSE(executor.Next(result));

    executor.Close();

    EXPECT_EQ(GetAffectedRows(savedResult), 1);
}

// ============================================================
// 16. Reject rows without source references
// ============================================================

TEST_F(DeleteExecutorTest, RejectsMissingSourceReference)
{
    auto valuesSchema =
        std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64},
                Field{"name", FieldType::STRING32},
                Field{"score", FieldType::INT64}
            }
        );

    auto rows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{2},
                    std::string{"Bob"},
                    int64_t{75}
                })}
            }
        );

    auto child = std::make_unique<ValuesExecutor>(
        rows,
        valuesSchema
    );

    DeleteExecutor executor(
        database,
        "students",
        std::move(child)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::exception
    );

    EXPECT_EQ(GetRemainingIDs().size(), 5);

    executor.Close();
}

// ============================================================
// 17. Reject source references from another table
// ============================================================

TEST_F(DeleteExecutorTest, RejectsWrongTableReference)
{
    auto valuesSchema =
        std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64}
            }
        );

    auto rows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{
                    Tuple({int64_t{2}}),
                    SourceReference{
                        "other_table",
                        insertedRIDs[1]
                    }
                }
            }
        );

    auto child = std::make_unique<ValuesExecutor>(
        rows,
        valuesSchema
    );

    DeleteExecutor executor(
        database,
        "students",
        std::move(child)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::exception
    );

    EXPECT_EQ(GetRemainingIDs().size(), 5);

    executor.Close();
}

// ============================================================
// 18. Reject duplicate target RIDs before writing
// ============================================================

TEST_F(DeleteExecutorTest, RejectsDuplicateTargetReferences)
{
    auto valuesSchema =
        std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64},
                Field{"name", FieldType::STRING32},
                Field{"score", FieldType::INT64}
            }
        );

    Tuple bob = database->readTuple(
        "students",
        insertedRIDs[1]
    );

    Row bobRow{
        bob,
        SourceReference{
            "students",
            insertedRIDs[1]
        }
    };

    auto rows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                bobRow,
                bobRow
            }
        );

    DeleteExecutor executor(
        database,
        "students",
        std::make_unique<ValuesExecutor>(
            rows,
            valuesSchema
        )
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::exception
    );

    // Duplicate references must be detected before writes.
    EXPECT_EQ(GetRemainingIDs().size(), 5);

    // Bob must still be readable.
    EXPECT_NO_THROW(
        database->readTuple("students", insertedRIDs[1])
    );

    executor.Close();
}
