
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "projection_executor.hpp"
#include "values_executor.hpp"

#include "constant_expression.hpp"
#include "column_expression.hpp"
#include "addition_expression.hpp"
#include "subtraction_expression.hpp"

// ============================================================
// Test Fixture
// ============================================================

class ProjectionExecutorTest : public ::testing::Test
{
protected:
    std::shared_ptr<const QuerySchema> schema;
    std::shared_ptr<const std::vector<Row>> rows;

    void SetUp() override
    {
        schema = std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64},
                Field{"name", FieldType::STRING32},
                Field{"score", FieldType::INT64}
            }
        );

        rows = std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
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
                })},
                Row{Tuple({
                    int64_t{4},
                    std::string{"Dan"},
                    int64_t{75}
                })},
                Row{Tuple({
                    int64_t{5},
                    std::string{"Eve"},
                    int64_t{90}
                })}
            }
        );
    }

    std::unique_ptr<Executor> MakeValuesExecutor()
    {
        return std::make_unique<ValuesExecutor>(
            rows,
            schema
        );
    }

    static ProjectionColumn Column(
        const std::string& name,
        FieldType type,
        std::size_t index
    )
    {
        return ProjectionColumn{
            Field{name, type},
            std::make_unique<ColumnExpression>(index)
        };
    }

    static ProjectionColumn Constant(
        const std::string& name,
        FieldType type,
        ExpressionVariant value
    )
    {
        return ProjectionColumn{
            Field{name, type},
            std::make_unique<ConstantExpression>(
                std::move(value)
            )
        };
    }

    static std::vector<Row> CollectRows(Executor& executor)
    {
        std::vector<Row> results;
        Row row;

        while (executor.Next(row))
        {
            results.push_back(row);
        }

        return results;
    }

    static void ExpectValues(
        const Row& row,
        const std::vector<FieldVariant>& expected
    )
    {
        EXPECT_EQ(
            row.tuple.getTupleValues(),
            expected
        );
    }

    static void ExpectSchema(
        const QuerySchema& actualSchema,
        const std::vector<Field>& expected
    )
    {
        const auto& actual = actualSchema.getFieldList();

        ASSERT_EQ(actual.size(), expected.size());

        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            EXPECT_EQ(actual[i].name, expected[i].name);
            EXPECT_EQ(actual[i].type, expected[i].type);
        }
    }
};

// ============================================================
// 1. Select a single column
// SELECT name FROM students
// ============================================================

TEST_F(ProjectionExecutorTest, SelectSingleColumn)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(results[0], {std::string{"Alice"}});
    ExpectValues(results[1], {std::string{"Bob"}});
    ExpectValues(results[2], {std::string{"Carol"}});
    ExpectValues(results[3], {std::string{"Dan"}});
    ExpectValues(results[4], {std::string{"Eve"}});

    executor.Close();
}

// ============================================================
// 2. Select multiple columns
// SELECT id, name FROM students
// ============================================================

TEST_F(ProjectionExecutorTest, SelectMultipleColumns)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("id", FieldType::INT64, 0)
    );

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(results[0], {int64_t{1}, std::string{"Alice"}});
    ExpectValues(results[1], {int64_t{2}, std::string{"Bob"}});
    ExpectValues(results[2], {int64_t{3}, std::string{"Carol"}});
    ExpectValues(results[3], {int64_t{4}, std::string{"Dan"}});
    ExpectValues(results[4], {int64_t{5}, std::string{"Eve"}});

    executor.Close();
}

// ============================================================
// 3. Reorder columns
// SELECT score, name, id FROM students
// ============================================================

TEST_F(ProjectionExecutorTest, ReordersColumns)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("score", FieldType::INT64, 2)
    );

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    projections.push_back(
        Column("id", FieldType::INT64, 0)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(
        results[0],
        {int64_t{90}, std::string{"Alice"}, int64_t{1}}
    );

    ExpectValues(
        results[1],
        {int64_t{75}, std::string{"Bob"}, int64_t{2}}
    );

    ExpectValues(
        results[2],
        {int64_t{85}, std::string{"Carol"}, int64_t{3}}
    );

    ExpectValues(
        results[3],
        {int64_t{75}, std::string{"Dan"}, int64_t{4}}
    );

    ExpectValues(
        results[4],
        {int64_t{90}, std::string{"Eve"}, int64_t{5}}
    );

    executor.Close();
}

// ============================================================
// 4. Rename a column
// SELECT name AS student_name FROM students
// ============================================================

TEST_F(ProjectionExecutorTest, RenamesColumn)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("student_name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    ExpectSchema(
        executor.OutputSchema(),
        {
            Field{"student_name", FieldType::STRING32}
        }
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(results[0], {std::string{"Alice"}});

    executor.Close();
}

// ============================================================
// 5. Computed addition
// SELECT name, score + 5 AS adjusted_score
// ============================================================

TEST_F(ProjectionExecutorTest, AdditionExpression)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    projections.push_back({
        Field{"adjusted_score", FieldType::INT64},
        std::make_unique<::AdditionExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{5})
        )
    });

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    ExpectSchema(
        executor.OutputSchema(),
        {
            Field{"name", FieldType::STRING32},
            Field{"adjusted_score", FieldType::INT64}
        }
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(results[0], {std::string{"Alice"}, int64_t{95}});
    ExpectValues(results[1], {std::string{"Bob"}, int64_t{80}});
    ExpectValues(results[2], {std::string{"Carol"}, int64_t{90}});
    ExpectValues(results[3], {std::string{"Dan"}, int64_t{80}});
    ExpectValues(results[4], {std::string{"Eve"}, int64_t{95}});

    executor.Close();
}

// ============================================================
// 6. Computed subtraction
// SELECT score - 10 AS adjusted_score
// ============================================================

TEST_F(ProjectionExecutorTest, SubtractionExpression)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back({
        Field{"adjusted_score", FieldType::INT64},
        std::make_unique<::SubtractionExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{10})
        )
    });

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(results[0], {int64_t{80}});
    ExpectValues(results[1], {int64_t{65}});
    ExpectValues(results[2], {int64_t{75}});
    ExpectValues(results[3], {int64_t{65}});
    ExpectValues(results[4], {int64_t{80}});

    executor.Close();
}

// ============================================================
// 7. Project a constant
// SELECT name, 100 AS max_score
// ============================================================

TEST_F(ProjectionExecutorTest, ProjectsConstant)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    projections.push_back(
        Constant("max_score", FieldType::INT64, int64_t{100})
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    for (std::size_t i = 0; i < results.size(); ++i)
    {
        const auto& values = results[i].tuple.getTupleValues();

        ASSERT_EQ(values.size(), 2);
        ASSERT_TRUE(std::holds_alternative<int64_t>(values[1]));

        EXPECT_EQ(std::get<int64_t>(values[1]), 100);
    }

    executor.Close();
}

// ============================================================
// 8. Duplicate projected columns
// SELECT score AS score1, score AS score2
// ============================================================

TEST_F(ProjectionExecutorTest, PreservesDuplicateColumns)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("score1", FieldType::INT64, 2)
    );

    projections.push_back(
        Column("score2", FieldType::INT64, 2)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(results[0], {int64_t{90}, int64_t{90}});
    ExpectValues(results[1], {int64_t{75}, int64_t{75}});
    ExpectValues(results[2], {int64_t{85}, int64_t{85}});
    ExpectValues(results[3], {int64_t{75}, int64_t{75}});
    ExpectValues(results[4], {int64_t{90}, int64_t{90}});

    executor.Close();
}

// ============================================================
// 9. Duplicate input rows are preserved
// ============================================================

TEST_F(ProjectionExecutorTest, PreservesDuplicateRows)
{
    auto duplicateRows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    int64_t{90}
                })},
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    int64_t{90}
                })}
            }
        );

    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        std::make_unique<ValuesExecutor>(
            duplicateRows,
            schema
        ),
        std::move(projections)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 2);

    ExpectValues(results[0], {std::string{"Alice"}});
    ExpectValues(results[1], {std::string{"Alice"}});

    executor.Close();
}

// ============================================================
// 10. Empty child retains projected schema
// ============================================================

TEST_F(ProjectionExecutorTest, EmptyChildHasSchema)
{
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        std::make_unique<ValuesExecutor>(
            emptyRows,
            schema
        ),
        std::move(projections)
    );

    ExpectSchema(
        executor.OutputSchema(),
        {
            Field{"name", FieldType::STRING32}
        }
    );

    executor.Init();

    Row result;

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ============================================================
// 11. Init resets iteration
// ============================================================

TEST_F(ProjectionExecutorTest, InitResetsIteration)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    auto firstResults = CollectRows(executor);

    executor.Close();
    executor.Init();

    auto secondResults = CollectRows(executor);

    ASSERT_EQ(firstResults.size(), 5);
    ASSERT_EQ(secondResults.size(), 5);

    for (std::size_t i = 0; i < firstResults.size(); ++i)
    {
        EXPECT_EQ(
            firstResults[i].tuple.getTupleValues(),
            secondResults[i].tuple.getTupleValues()
        );
    }

    executor.Close();
}

// ============================================================
// 12. Close before exhaustion
// ============================================================

TEST_F(ProjectionExecutorTest, CloseBeforeExhaustion)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("id", FieldType::INT64, 0)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));
    ExpectValues(result, {int64_t{1}});

    executor.Close();

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    executor.Close();
}

// ============================================================
// 13. Close is idempotent
// ============================================================

TEST_F(ProjectionExecutorTest, CloseIsIdempotent)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("id", FieldType::INT64, 0)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 5);

    executor.Close();
}

// ============================================================
// 14. End of input remains exhausted
// ============================================================

TEST_F(ProjectionExecutorTest, ExhaustionReturnsFalse)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("id", FieldType::INT64, 0)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    Row result;

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_TRUE(executor.Next(result));
    }

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ============================================================
// 15. Returned rows remain valid
// ============================================================

TEST_F(ProjectionExecutorTest, ReturnedRowsRemainValid)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    Row firstRow;
    Row secondRow;

    ASSERT_TRUE(executor.Next(firstRow));

    const auto savedValues =
        firstRow.tuple.getTupleValues();

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
}

// ============================================================
// 16. Projection removes source references
// ============================================================

TEST_F(ProjectionExecutorTest, RemovesSourceReference)
{
    // Populate an actual source reference in the input row.
    // RID is aggregate-initialized from PageID and tupleSlot.
    auto referencedRows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{
                    Tuple({
                        int64_t{1},
                        std::string{"Alice"},
                        int64_t{90}
                    }),
                    SourceReference{
                        "students",
                        RID{PageID{0, 1}, 0}
                    }
                }
            }
        );

    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("name", FieldType::STRING32, 1)
    );

    ProjectionExecutor executor(
        std::make_unique<ValuesExecutor>(
            referencedRows,
            schema
        ),
        std::move(projections)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    ExpectValues(result, {std::string{"Alice"}});
    EXPECT_FALSE(result.source.has_value());

    executor.Close();
}

// ============================================================
// 17. Invalid column index
// ============================================================

TEST_F(ProjectionExecutorTest, RejectsInvalidColumnIndex)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back(
        Column("invalid", FieldType::INT64, 100)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::out_of_range
    );

    executor.Close();
}

// ============================================================
// 18. Declared type does not match expression result
// ============================================================

TEST_F(ProjectionExecutorTest, RejectsTypeMismatch)
{
    std::vector<ProjectionColumn> projections;

    // Column 1 contains STRING32, but output declares INT64.
    projections.push_back(
        Column("invalid", FieldType::INT64, 1)
    );

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::invalid_argument
    );

    executor.Close();
}

// ============================================================
// 19. Boolean expression cannot be projected
// ============================================================

TEST_F(ProjectionExecutorTest, RejectsBooleanProjection)
{
    std::vector<ProjectionColumn> projections;

    projections.push_back({
        Field{"invalid", FieldType::INT64},
        std::make_unique<ConstantExpression>(true)
    });

    ProjectionExecutor executor(
        MakeValuesExecutor(),
        std::move(projections)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::invalid_argument
    );

    executor.Close();
}

// ============================================================
// 20. Arithmetic overflow propagates
// ============================================================

TEST_F(ProjectionExecutorTest, PropagatesArithmeticOverflow)
{
    auto overflowRows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    std::numeric_limits<int64_t>::max()
                })}
            }
        );

    std::vector<ProjectionColumn> projections;

    projections.push_back({
        Field{"adjusted_score", FieldType::INT64},
        std::make_unique<::AdditionExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{1})
        )
    });

    ProjectionExecutor executor(
        std::make_unique<ValuesExecutor>(
            overflowRows,
            schema
        ),
        std::move(projections)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(
        executor.Next(result),
        std::overflow_error
    );

    executor.Close();
}
