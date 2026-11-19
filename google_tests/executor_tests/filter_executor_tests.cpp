
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "filter_executor.hpp"
#include "values_executor.hpp"

#include "constant_expression.hpp"
#include "column_expression.hpp"
#include "equal_expression.hpp"
#include "greater_than_expression.hpp"
#include "greater_than_equal_expression.hpp"
#include "less_than_expression.hpp"
#include "and_expression.hpp"
#include "or_expression.hpp"
#include "not_expression.hpp"

// ============================================================
// Test Fixture
// ============================================================

class FilterExecutorTest : public ::testing::Test
{
protected:
    std::shared_ptr<const Schema> schema;
    std::shared_ptr<const std::vector<Row>> rows;

    void SetUp() override
    {
        schema = std::make_shared<const Schema>(
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

    static int64_t GetID(const Row& row)
    {
        return std::get<int64_t>(
            row.tuple.getTupleValues().at(0)
        );
    }

    static int64_t GetScore(const Row& row)
    {
        return std::get<int64_t>(
            row.tuple.getTupleValues().at(2)
        );
    }

    static void ExpectIDs(
        const std::vector<Row>& rows,
        const std::vector<int64_t>& expected
    )
    {
        std::vector<int64_t> actual;

        for (const auto& row : rows)
        {
            actual.push_back(GetID(row));
        }

        EXPECT_EQ(actual, expected);
    }
};

// ============================================================
// Test 1: Greater Than Predicate
// score > 85
// Expected: Alice, Eve
// ============================================================

TEST_F(FilterExecutorTest, GreaterThanPredicate)
{
    auto predicate =
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{85})
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {1, 5});

    executor.Close();
}

// ============================================================
// Test 2: Greater Than Or Equal Predicate
// score >= 85
// Expected: Alice, Carol, Eve
// ============================================================

TEST_F(FilterExecutorTest, GreaterEqualPredicate)
{
    auto predicate =
        std::make_unique<GreaterThanEqualExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{85})
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {1, 3, 5});

    executor.Close();
}

// ============================================================
// Test 3: Less Than Predicate
// score < 85
// Expected: Bob, Dan
// ============================================================

TEST_F(FilterExecutorTest, LessThanPredicate)
{
    auto predicate =
        std::make_unique<LessThanExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{85})
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {2, 4});

    executor.Close();
}

// ============================================================
// Test 4: Equality Predicate
// score == 90
// Expected: Alice, Eve
// ============================================================

TEST_F(FilterExecutorTest, EqualPredicate)
{
    auto predicate =
        std::make_unique<EqualExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{90})
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {1, 5});

    executor.Close();
}

// ============================================================
// Test 5: String Equality
// name == "Bob"
// ============================================================

TEST_F(FilterExecutorTest, StringEquality)
{
    auto predicate =
        std::make_unique<EqualExpression>(
            std::make_unique<ColumnExpression>(1),
            std::make_unique<ConstantExpression>(
                std::string{"Bob"}
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {2});

    executor.Close();
}

// ============================================================
// Test 6: AND Predicate
// score >= 85 AND id > 3
// Expected: Eve
// ============================================================

TEST_F(FilterExecutorTest, AndPredicate)
{
    auto predicate =
        std::make_unique<AndExpression>(
            std::make_unique<GreaterThanEqualExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(
                    int64_t{85}
                )
            ),
            std::make_unique<GreaterThanExpression>(
                std::make_unique<ColumnExpression>(0),
                std::make_unique<ConstantExpression>(
                    int64_t{3}
                )
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {5});

    executor.Close();
}

// ============================================================
// Test 7: OR Predicate
// score == 75 OR score == 85
// Expected: Bob, Carol, Dan
// ============================================================

TEST_F(FilterExecutorTest, OrPredicate)
{
    auto predicate =
        std::make_unique<OrExpression>(
            std::make_unique<EqualExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(
                    int64_t{75}
                )
            ),
            std::make_unique<EqualExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(
                    int64_t{85}
                )
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {2, 3, 4});

    executor.Close();
}

// ============================================================
// Test 8: NOT Predicate
// NOT(score == 90)
// Expected: Bob, Carol, Dan
// ============================================================

TEST_F(FilterExecutorTest, NotPredicate)
{
    auto predicate =
        std::make_unique<NotExpression>(
            std::make_unique<EqualExpression>(
                std::make_unique<ColumnExpression>(2),
                std::make_unique<ConstantExpression>(
                    int64_t{90}
                )
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {2, 3, 4});

    executor.Close();
}

// ============================================================
// Test 9: Predicate matches no rows
// score > 100
// ============================================================

TEST_F(FilterExecutorTest, NoMatchingRows)
{
    auto predicate =
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(
                int64_t{100}
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// Test 10: Predicate matches every row
// score >= 0
// ============================================================

TEST_F(FilterExecutorTest, AllRowsMatch)
{
    auto predicate =
        std::make_unique<GreaterThanEqualExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(
                int64_t{0}
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {1, 2, 3, 4, 5});

    executor.Close();
}

// ============================================================
// Test 11: Empty child executor
// ============================================================

TEST_F(FilterExecutorTest, EmptyChild)
{
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    auto child = std::make_unique<ValuesExecutor>(
        emptyRows,
        schema
    );

    auto predicate =
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(
                int64_t{50}
            )
        );

    FilterExecutor executor(
        std::move(child),
        std::move(predicate)
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// Test 12: Output schema matches child schema
// ============================================================

TEST_F(FilterExecutorTest, OutputSchemaMatchesChild)
{
    auto predicate =
        std::make_unique<ConstantExpression>(true);

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    const Schema& outputSchema = executor.OutputSchema();

    const auto& fields = outputSchema.getFieldList();

    ASSERT_EQ(fields.size(), 3);

    EXPECT_EQ(fields[0].name, "id");
    EXPECT_EQ(fields[0].type, FieldType::INT64);

    EXPECT_EQ(fields[1].name, "name");
    EXPECT_EQ(fields[1].type, FieldType::STRING32);

    EXPECT_EQ(fields[2].name, "score");
    EXPECT_EQ(fields[2].type, FieldType::INT64);
}

// ============================================================
// Test 13: Init resets the filter
// ============================================================

TEST_F(FilterExecutorTest, InitResetsIteration)
{
    auto predicate =
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(
                int64_t{85}
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto firstResults = CollectRows(executor);

    executor.Close();

    executor.Init();

    auto secondResults = CollectRows(executor);

    ExpectIDs(firstResults, {1, 5});
    ExpectIDs(secondResults, {1, 5});

    executor.Close();
}

// ============================================================
// Test 14: Close before exhaustion
// ============================================================

TEST_F(FilterExecutorTest, CloseBeforeExhaustion)
{
    auto predicate =
        std::make_unique<ConstantExpression>(true);

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));
    EXPECT_EQ(GetID(row), 1);

    executor.Close();

    executor.Init();

    auto results = CollectRows(executor);

    ExpectIDs(results, {1, 2, 3, 4, 5});

    executor.Close();
}

// ============================================================
// Test 15: Close is idempotent
// ============================================================

TEST_F(FilterExecutorTest, CloseIsIdempotent)
{
    auto predicate =
        std::make_unique<ConstantExpression>(true);

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
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
// Test 16: Exhaustion remains exhausted
// ============================================================

TEST_F(FilterExecutorTest, ExhaustionReturnsFalse)
{
    auto predicate =
        std::make_unique<ConstantExpression>(true);

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    Row row;

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_TRUE(executor.Next(row));
    }

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// Test 17: Returned rows retain their values
// ============================================================

TEST_F(FilterExecutorTest, ReturnedRowsRemainValid)
{
    auto predicate =
        std::make_unique<ConstantExpression>(true);

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    Row firstRow;
    Row secondRow;

    ASSERT_TRUE(executor.Next(firstRow));

    auto savedValues =
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
// Test 18: Reject non-Boolean predicates
// ============================================================

TEST_F(FilterExecutorTest, RejectsNonBooleanPredicate)
{
    auto predicate =
        std::make_unique<ConstantExpression>(
            int64_t{10}
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    Row row;

    EXPECT_THROW(
        executor.Next(row),
        std::invalid_argument
    );

    executor.Close();
}

// ============================================================
// Test 19: Reject invalid column indexes
// ============================================================

TEST_F(FilterExecutorTest, RejectsInvalidColumnIndex)
{
    auto predicate =
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(100),
            std::make_unique<ConstantExpression>(
                int64_t{50}
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    Row row;

    EXPECT_THROW(
        executor.Next(row),
        std::out_of_range
    );

    executor.Close();
}

// ============================================================
// Test 20: Reject incompatible operand types
// ============================================================

TEST_F(FilterExecutorTest, RejectsIncompatibleTypes)
{
    auto predicate =
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(1),
            std::make_unique<ConstantExpression>(
                int64_t{50}
            )
        );

    FilterExecutor executor(
        MakeValuesExecutor(),
        std::move(predicate)
    );

    executor.Init();

    Row row;

    EXPECT_THROW(
        executor.Next(row),
        std::invalid_argument
    );

    executor.Close();
}
