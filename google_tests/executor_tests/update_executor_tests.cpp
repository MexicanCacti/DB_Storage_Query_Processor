
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

#include "update_executor.hpp"
#include "sequential_scan_executor.hpp"
#include "filter_executor.hpp"
#include "values_executor.hpp"

#include "column_expression.hpp"
#include "constant_expression.hpp"
#include "equal_expression.hpp"
#include "less_than_expression.hpp"
#include "addition_expression.hpp"

namespace fs = std::filesystem;

// ============================================================
// Test Fixture
// ============================================================

class UpdateExecutorTest : public ::testing::Test
{
protected:
    fs::path databaseDirectory;
    std::shared_ptr<Database> database;
    std::vector<RID> insertedRIDs;

    void SetUp() override
    {
        databaseDirectory =
            fs::temp_directory_path() /
            ("minidb_update_" +
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

    std::unique_ptr<Executor> MakeFilterByScoreLessThan(
        int64_t score
    )
    {
        return std::make_unique<FilterExecutor>(
            MakeScan(),
            std::make_unique<LessThanExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(score)
            )
        );
    }

    static UpdateExpression SetScore(int64_t value)
    {
        return UpdateExpression{
            2,
            std::make_unique<ConstantExpression>(value)
        };
    }

    static UpdateExpression AddToScore(int64_t value)
    {
        return UpdateExpression{
            2,
            std::make_unique<AdditionExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(value)
            )
        };
    }

    static UpdateExpression SetName(const std::string& name)
    {
        return UpdateExpression{
            1,
            std::make_unique<ConstantExpression>(name)
        };
    }

    static std::vector<UpdateExpression> ScoreAssignments(
        int64_t increment
    )
    {
        std::vector<UpdateExpression> assignments;

        assignments.push_back(AddToScore(increment));

        return assignments;
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

    Tuple ReadStudent(std::size_t index)
    {
        return database->readTuple(
            "students",
            insertedRIDs.at(index)
        );
    }

    static int64_t GetScore(const Tuple& tuple)
    {
        return std::get<int64_t>(
            tuple.getTupleValues().at(2)
        );
    }

    static int64_t GetID(const Tuple& tuple)
    {
        return std::get<int64_t>(
            tuple.getTupleValues().at(0)
        );
    }

    static std::string GetName(const Tuple& tuple)
    {
        return std::get<std::string>(
            tuple.getTupleValues().at(1)
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

    std::unique_ptr<Executor> MakeReferencedRows(
        std::vector<Row> rows
    )
    {
        auto querySchema =
            std::make_shared<const QuerySchema>(
                std::vector<Field>{
                    Field{"id", FieldType::INT64},
                    Field{"name", FieldType::STRING32},
                    Field{"score", FieldType::INT64}
                }
            );

        auto rowPointer =
            std::make_shared<const std::vector<Row>>(
                std::move(rows)
            );

        return std::make_unique<ValuesExecutor>(
            rowPointer,
            querySchema
        );
    }
};

// ============================================================
// 1. Update a single row
// ============================================================

TEST_F(UpdateExecutorTest, UpdatesSingleRow)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 1);

    Tuple bob = ReadStudent(1);

    EXPECT_EQ(GetID(bob), 2);
    EXPECT_EQ(GetName(bob), "Bob");
    EXPECT_EQ(GetScore(bob), 85);

    executor.Close();
}

// ============================================================
// 2. Update multiple qualifying rows
// ============================================================

TEST_F(UpdateExecutorTest, UpdatesMultipleRows)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByScoreLessThan(80),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 2);

    EXPECT_EQ(GetScore(ReadStudent(0)), 90);
    EXPECT_EQ(GetScore(ReadStudent(1)), 85);
    EXPECT_EQ(GetScore(ReadStudent(2)), 85);
    EXPECT_EQ(GetScore(ReadStudent(3)), 85);
    EXPECT_EQ(GetScore(ReadStudent(4)), 90);

    executor.Close();
}

// ============================================================
// 3. Update all rows
// ============================================================

TEST_F(UpdateExecutorTest, UpdatesAllRows)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeScan(),
        ScoreAssignments(5)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 5);

    EXPECT_EQ(GetScore(ReadStudent(0)), 95);
    EXPECT_EQ(GetScore(ReadStudent(1)), 80);
    EXPECT_EQ(GetScore(ReadStudent(2)), 90);
    EXPECT_EQ(GetScore(ReadStudent(3)), 80);
    EXPECT_EQ(GetScore(ReadStudent(4)), 95);

    executor.Close();
}

// ============================================================
// 4. No matching rows
// ============================================================

TEST_F(UpdateExecutorTest, NoMatchingRows)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(100),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 0);

    EXPECT_FALSE(executor.Next(result));

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 5. Update a string column
// ============================================================

TEST_F(UpdateExecutorTest, UpdatesStringColumn)
{
    std::vector<UpdateExpression> assignments;
    assignments.push_back(SetName("Robert"));

    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        std::move(assignments)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 1);

    EXPECT_EQ(GetName(ReadStudent(1)), "Robert");
    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 6. Multiple assignments use original row values
//
// Original Bob: id = 2, score = 75
// SET id = score + 1, score = id + 10
// Expected: id = 76, score = 12
// ============================================================

TEST_F(UpdateExecutorTest, MultipleAssignmentsUseOriginalRow)
{
    std::vector<UpdateExpression> assignments;

    assignments.push_back({
        0,
        std::make_unique<AdditionExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{1})
        )
    });

    assignments.push_back({
        2,
        std::make_unique<AdditionExpression>(
            std::make_unique<ColumnExpression>(0),
            std::make_unique<ConstantExpression>(int64_t{10})
        )
    });

    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        std::move(assignments)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 1);

    Tuple updated = ReadStudent(1);

    EXPECT_EQ(GetID(updated), 76);
    EXPECT_EQ(GetScore(updated), 12);

    executor.Close();
}

// ============================================================
// 7. Unassigned columns remain unchanged
// ============================================================

TEST_F(UpdateExecutorTest, UnassignedColumnsRemainUnchanged)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    Tuple bob = ReadStudent(1);

    EXPECT_EQ(GetID(bob), 2);
    EXPECT_EQ(GetName(bob), "Bob");
    EXPECT_EQ(GetScore(bob), 85);

    executor.Close();
}

// ============================================================
// 8. Updated record preserves its RID
// ============================================================

TEST_F(UpdateExecutorTest, PreservesRID)
{
    const RID originalRID = insertedRIDs[1];

    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    auto scannedRows = database->scan("students");

    bool foundBob = false;

    for (const auto& [tuple, rid] : scannedRows)
    {
        if (GetID(tuple) == 2)
        {
            ExpectRIDEqual(rid, originalRID);
            foundBob = true;
        }
    }

    EXPECT_TRUE(foundBob);

    executor.Close();
}

// ============================================================
// 9. Exactly one affected-rows result
// ============================================================

TEST_F(UpdateExecutorTest, ReturnsExactlyOneCountRow)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_EQ(GetAffectedRows(result), 1);

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    EXPECT_EQ(GetScore(ReadStudent(1)), 85);

    executor.Close();
}

// ============================================================
// 10. Init and Close without Next perform no writes
// ============================================================

TEST_F(UpdateExecutorTest, InitAndCloseDoNotModifyStorage)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    executor.Init();

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);
}

// ============================================================
// 11. Reinitialization performs a new update
// ============================================================

TEST_F(UpdateExecutorTest, ReinitPerformsNewUpdate)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    Row result;

    executor.Init();
    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 1);
    EXPECT_EQ(GetScore(ReadStudent(1)), 85);

    executor.Close();

    executor.Init();
    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 1);
    EXPECT_EQ(GetScore(ReadStudent(1)), 95);

    executor.Close();
}

// ============================================================
// 12. Output schema
// ============================================================

TEST_F(UpdateExecutorTest, OutputSchemaIsAffectedRows)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    const auto& fields =
        executor.OutputSchema().getFieldList();

    ASSERT_EQ(fields.size(), 1);

    // Matches your current header.
    EXPECT_EQ(fields[0].name, "Affected Rows");
    EXPECT_EQ(fields[0].type, FieldType::INT64);
}

// ============================================================
// 13. Count row has no source reference
// ============================================================

TEST_F(UpdateExecutorTest, CountRowHasNoSourceReference)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    EXPECT_FALSE(result.source.has_value());

    executor.Close();
}

// ============================================================
// 14. Close is idempotent
// ============================================================

TEST_F(UpdateExecutorTest, CloseIsIdempotent)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        ScoreAssignments(10)
    );

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    EXPECT_EQ(GetScore(ReadStudent(1)), 85);
}

// ============================================================
// 15. Reject invalid target column index
// ============================================================

TEST_F(UpdateExecutorTest, RejectsInvalidColumnIndex)
{
    std::vector<UpdateExpression> assignments;

    assignments.push_back({
        100,
        std::make_unique<ConstantExpression>(int64_t{10})
    });

    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        std::move(assignments)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::exception);

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 16. Reject incompatible assignment type
// ============================================================

TEST_F(UpdateExecutorTest, RejectsTypeMismatch)
{
    std::vector<UpdateExpression> assignments;

    assignments.push_back({
        2,
        std::make_unique<ConstantExpression>(
            std::string{"InvalidScore"}
        )
    });

    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        std::move(assignments)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::exception);

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 17. Reject Boolean assignment
// ============================================================

TEST_F(UpdateExecutorTest, RejectsBooleanAssignment)
{
    std::vector<UpdateExpression> assignments;

    assignments.push_back({
        2,
        std::make_unique<ConstantExpression>(true)
    });

    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        std::move(assignments)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::exception);

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 18. Reject oversized STRING32
// ============================================================

TEST_F(UpdateExecutorTest, RejectsOversizedString)
{
    std::vector<UpdateExpression> assignments;

    assignments.push_back(
        SetName(std::string(33, 'A'))
    );

    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByID(2),
        std::move(assignments)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::exception);

    EXPECT_EQ(GetName(ReadStudent(1)), "Bob");

    executor.Close();
}

// ============================================================
// 19. Reject missing source reference
// ============================================================

TEST_F(UpdateExecutorTest, RejectsMissingSourceReference)
{
    Row bobRow{
        ReadStudent(1),
        std::nullopt
    };

    UpdateExecutor executor(
        database,
        "students",
        MakeReferencedRows({bobRow}),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::exception);

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 20. Reject source from another table
// ============================================================

TEST_F(UpdateExecutorTest, RejectsWrongTableReference)
{
    Row bobRow{
        ReadStudent(1),
        SourceReference{
            "other_table",
            insertedRIDs[1]
        }
    };

    UpdateExecutor executor(
        database,
        "students",
        MakeReferencedRows({bobRow}),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::exception);

    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 21. Reject duplicate target RIDs before writing
// ============================================================

TEST_F(UpdateExecutorTest, RejectsDuplicateTargetReferences)
{
    Row bobRow{
        ReadStudent(1),
        SourceReference{
            "students",
            insertedRIDs[1]
        }
    };

    UpdateExecutor executor(
        database,
        "students",
        MakeReferencedRows({bobRow, bobRow}),
        ScoreAssignments(10)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::exception);

    // No partial updates.
    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}

// ============================================================
// 22. Updating a filter column does not revisit records
// ============================================================

TEST_F(UpdateExecutorTest, UpdatingFilterColumnDoesNotRevisitRows)
{
    UpdateExecutor executor(
        database,
        "students",
        MakeFilterByScoreLessThan(90),
        ScoreAssignments(20)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    EXPECT_EQ(GetAffectedRows(result), 3);

    EXPECT_EQ(GetScore(ReadStudent(0)), 90);
    EXPECT_EQ(GetScore(ReadStudent(1)), 95);
    EXPECT_EQ(GetScore(ReadStudent(2)), 105);
    EXPECT_EQ(GetScore(ReadStudent(3)), 95);
    EXPECT_EQ(GetScore(ReadStudent(4)), 90);

    executor.Close();
}

// ============================================================
// 23. Invalid later update prevents earlier writes
// ============================================================

TEST_F(UpdateExecutorTest, ValidatesAllUpdatesBeforeWriting)
{
    // Alice's score is 90, Bob's is 75.
    // A deliberately invalid later assignment must prevent
    // earlier rows from being modified.

    std::vector<UpdateExpression> assignments;

    assignments.push_back({
        2,
        std::make_unique<AdditionExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{10})
        )
    });

    // Force overflow for every row with a sufficiently
    // large score, by constructing a separate input below.

    auto invalidRows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{
                    ReadStudent(1),
                    SourceReference{"students", insertedRIDs[1]}
                },
                Row{
                    Tuple({
                        int64_t{1},
                        std::string{"Alice"},
                        std::numeric_limits<int64_t>::max()
                    }),
                    SourceReference{"students", insertedRIDs[0]}
                }
            }
        );

    auto querySchema =
        std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64},
                Field{"name", FieldType::STRING32},
                Field{"score", FieldType::INT64}
            }
        );

    UpdateExecutor executor(
        database,
        "students",
        std::make_unique<ValuesExecutor>(
            invalidRows,
            querySchema
        ),
        std::move(assignments)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::overflow_error);

    // Bob must remain unchanged even though his proposed
    // update was valid.
    EXPECT_EQ(GetScore(ReadStudent(1)), 75);

    executor.Close();
}
