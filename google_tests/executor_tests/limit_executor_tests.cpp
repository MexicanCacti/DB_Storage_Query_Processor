
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "limit_executor.hpp"
#include "values_executor.hpp"
#include "tuple.hpp"
#include "schema.hpp"
#include "types.hpp"

// ============================================================
// Test Fixture
// ============================================================

class LimitExecutorTest : public ::testing::Test
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
// 1. Limit smaller than input
// LIMIT 3
// ============================================================

TEST_F(LimitExecutorTest, LimitLessThanInputSize)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        3
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    ExpectValues(
        results[0],
        {int64_t{1}, std::string{"Alice"}, int64_t{90}}
    );

    ExpectValues(
        results[1],
        {int64_t{2}, std::string{"Bob"}, int64_t{75}}
    );

    ExpectValues(
        results[2],
        {int64_t{3}, std::string{"Carol"}, int64_t{85}}
    );

    executor.Close();
}

// ============================================================
// 2. Limit equal to input size
// LIMIT 5
// ============================================================

TEST_F(LimitExecutorTest, LimitEqualsInputSize)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        5
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    ExpectValues(
        results[0],
        {int64_t{1}, std::string{"Alice"}, int64_t{90}}
    );

    ExpectValues(
        results[4],
        {int64_t{5}, std::string{"Eve"}, int64_t{90}}
    );

    executor.Close();
}

// ============================================================
// 3. Limit greater than input size
// LIMIT 10
// ============================================================

TEST_F(LimitExecutorTest, LimitGreaterThanInputSize)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        10
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    executor.Close();
}

// ============================================================
// 4. Limit zero
// LIMIT 0
// ============================================================

TEST_F(LimitExecutorTest, LimitZero)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        0
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 5. Limit one
// LIMIT 1
// ============================================================

TEST_F(LimitExecutorTest, LimitOne)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        1
    );

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));

    ExpectValues(
        row,
        {int64_t{1}, std::string{"Alice"}, int64_t{90}}
    );

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 6. Empty child
// ============================================================

TEST_F(LimitExecutorTest, EmptyChild)
{
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    LimitExecutor executor(
        std::make_unique<ValuesExecutor>(
            emptyRows,
            schema
        ),
        3
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 7. Output schema matches child
// ============================================================

TEST_F(LimitExecutorTest, OutputSchemaMatchesChild)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        2
    );

    ExpectSchema(
        executor.OutputSchema(),
        {
            Field{"id", FieldType::INT64},
            Field{"name", FieldType::STRING32},
            Field{"score", FieldType::INT64}
        }
    );

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 2);

    executor.Close();
}

// ============================================================
// 8. Init resets iteration
// ============================================================

TEST_F(LimitExecutorTest, InitResetsIteration)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        3
    );

    executor.Init();

    auto firstResults = CollectRows(executor);

    executor.Close();

    executor.Init();

    auto secondResults = CollectRows(executor);

    ASSERT_EQ(firstResults.size(), 3);
    ASSERT_EQ(secondResults.size(), 3);

    for (std::size_t i = 0; i < 3; ++i)
    {
        EXPECT_EQ(
            firstResults[i].tuple.getTupleValues(),
            secondResults[i].tuple.getTupleValues()
        );
    }

    executor.Close();
}

// ============================================================
// 9. Close before exhaustion
// ============================================================

TEST_F(LimitExecutorTest, CloseBeforeExhaustion)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        4
    );

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));

    ExpectValues(
        row,
        {int64_t{1}, std::string{"Alice"}, int64_t{90}}
    );

    executor.Close();

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 4);

    executor.Close();
}

// ============================================================
// 10. Close is idempotent
// ============================================================

TEST_F(LimitExecutorTest, CloseIsIdempotent)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        3
    );

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 3);

    executor.Close();
}

// ============================================================
// 11. Exhaustion remains exhausted
// ============================================================

TEST_F(LimitExecutorTest, ExhaustionReturnsFalse)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        2
    );

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));
    ASSERT_TRUE(executor.Next(row));

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 12. Returned rows remain valid
// ============================================================

TEST_F(LimitExecutorTest, ReturnedRowsRemainValid)
{
    LimitExecutor executor(
        MakeValuesExecutor(),
        3
    );

    executor.Init();

    Row firstRow;
    Row secondRow;

    ASSERT_TRUE(executor.Next(firstRow));

    auto savedValues = firstRow.tuple.getTupleValues();

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
// 13. Limit preserves duplicate rows
// ============================================================

TEST_F(LimitExecutorTest, PreservesDuplicates)
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
                })},
                Row{Tuple({
                    int64_t{2},
                    std::string{"Bob"},
                    int64_t{75}
                })}
            }
        );

    LimitExecutor executor(
        std::make_unique<ValuesExecutor>(
            duplicateRows,
            schema
        ),
        2
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 2);

    EXPECT_EQ(
        results[0].tuple.getTupleValues(),
        results[1].tuple.getTupleValues()
    );

    executor.Close();
}

// ============================================================
// Counting Executor
//
// Tracks how many times Next() is called.
// Used to verify that Limit does not over-consume input.
// ============================================================

class CountingExecutor : public Executor
{
private:
    std::unique_ptr<Executor> childExecutor;
    std::shared_ptr<std::size_t> nextCallCount;

public:
    CountingExecutor(
        std::unique_ptr<Executor> child,
        std::shared_ptr<std::size_t> counter
    )
        : childExecutor(std::move(child)),
          nextCallCount(std::move(counter))
    {}

    void Init() override
    {
        childExecutor->Init();
        *nextCallCount = 0;
    }

    bool Next(Row& out) override
    {
        ++(*nextCallCount);
        return childExecutor->Next(out);
    }

    void Close() override
    {
        childExecutor->Close();
    }

    const QuerySchema& OutputSchema() const override
    {
        return childExecutor->OutputSchema();
    }
};

// ============================================================
// 14. Limit 0 never calls child Next()
// ============================================================

TEST_F(LimitExecutorTest, LimitZeroDoesNotReadChild)
{
    auto counter = std::make_shared<std::size_t>(0);

    auto countingChild = std::make_unique<CountingExecutor>(
        MakeValuesExecutor(),
        counter
    );

    LimitExecutor executor(
        std::move(countingChild),
        0
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    EXPECT_EQ(*counter, 0);

    executor.Close();
}

// ============================================================
// 15. Limit K does not read past K rows
// ============================================================

TEST_F(LimitExecutorTest, DoesNotReadPastLimit)
{
    auto counter = std::make_shared<std::size_t>(0);

    auto countingChild = std::make_unique<CountingExecutor>(
        MakeValuesExecutor(),
        counter
    );

    LimitExecutor executor(
        std::move(countingChild),
        3
    );

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));
    ASSERT_TRUE(executor.Next(row));
    ASSERT_TRUE(executor.Next(row));

    EXPECT_EQ(*counter, 3);

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    // No additional child Next() calls are allowed.
    EXPECT_EQ(*counter, 3);

    executor.Close();
}

// ============================================================
// 16. Limit greater than input may reach child exhaustion
// ============================================================

TEST_F(LimitExecutorTest, ReadsUntilChildExhaustion)
{
    auto counter = std::make_shared<std::size_t>(0);

    auto countingChild = std::make_unique<CountingExecutor>(
        MakeValuesExecutor(),
        counter
    );

    LimitExecutor executor(
        std::move(countingChild),
        10
    );

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 5);

    // Five successful reads and one exhaustion check.
    EXPECT_EQ(*counter, 6);

    executor.Close();
}
