
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "sort_executor.hpp"
#include "values_executor.hpp"
#include "column_expression.hpp"
#include "constant_expression.hpp"
#include "addition_expression.hpp"

// ============================================================
// Test Fixture
// ============================================================

class SortExecutorTest : public ::testing::Test
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
                    int64_t{3},
                    std::string{"Carol"},
                    int64_t{85}
                })},
                Row{Tuple({
                    int64_t{5},
                    std::string{"Eve"},
                    int64_t{90}
                })},
                Row{Tuple({
                    int64_t{2},
                    std::string{"Bob"},
                    int64_t{75}
                })},
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    int64_t{90}
                })},
                Row{Tuple({
                    int64_t{4},
                    std::string{"Dan"},
                    int64_t{75}
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

    static SortKey ColumnKey(
        std::size_t index,
        SortDirection direction
    )
    {
        return SortKey{
            std::make_unique<ColumnExpression>(index),
            direction
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

    static std::vector<int64_t> GetIDs(
        const std::vector<Row>& rows
    )
    {
        std::vector<int64_t> ids;

        for (const Row& row : rows)
        {
            ids.push_back(
                std::get<int64_t>(
                    row.tuple.getTupleValues().at(0)
                )
            );
        }

        return ids;
    }

    static std::vector<int64_t> GetScores(
        const std::vector<Row>& rows
    )
    {
        std::vector<int64_t> scores;

        for (const Row& row : rows)
        {
            scores.push_back(
                std::get<int64_t>(
                    row.tuple.getTupleValues().at(2)
                )
            );
        }

        return scores;
    }
};

// ============================================================
// 1. Sort by ID ascending
// ORDER BY id ASC
// ============================================================

TEST_F(SortExecutorTest, SortAscending)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );

    executor.Close();
}

// ============================================================
// 2. Sort by ID descending
// ORDER BY id DESC
// ============================================================

TEST_F(SortExecutorTest, SortDescending)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::DESC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{5, 4, 3, 2, 1})
    );

    executor.Close();
}

// ============================================================
// 3. Sort by score ascending
// ============================================================

TEST_F(SortExecutorTest, SortScoresAscending)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(2, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetScores(results),
        (std::vector<int64_t>{75, 75, 85, 90, 90})
    );

    executor.Close();
}

// ============================================================
// 4. Sort by score descending
// ============================================================

TEST_F(SortExecutorTest, SortScoresDescending)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(2, SortDirection::DESC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetScores(results),
        (std::vector<int64_t>{90, 90, 85, 75, 75})
    );

    executor.Close();
}

// ============================================================
// 5. Multiple sort keys
// ORDER BY score DESC, id ASC
//
// Expected IDs: 1, 5, 3, 2, 4
// ============================================================

TEST_F(SortExecutorTest, MultipleKeysMixedDirections)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(2, SortDirection::DESC)
    );

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{1, 5, 3, 2, 4})
    );

    executor.Close();
}

// ============================================================
// 6. Multiple sort keys
// ORDER BY score ASC, id DESC
//
// Expected IDs: 4, 2, 3, 5, 1
// ============================================================

TEST_F(SortExecutorTest, MultipleKeysAscendingDescending)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(2, SortDirection::ASC)
    );

    keys.push_back(
        ColumnKey(0, SortDirection::DESC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{4, 2, 3, 5, 1})
    );

    executor.Close();
}

// ============================================================
// 7. Sort by string column ascending
// ORDER BY name ASC
// ============================================================

TEST_F(SortExecutorTest, SortStringsAscending)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(1, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );

    executor.Close();
}

// ============================================================
// 8. Sort by string column descending
// ORDER BY name DESC
// ============================================================

TEST_F(SortExecutorTest, SortStringsDescending)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(1, SortDirection::DESC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{5, 4, 3, 2, 1})
    );

    executor.Close();
}

// ============================================================
// 9. Empty input
// ============================================================

TEST_F(SortExecutorTest, EmptyInput)
{
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        std::make_unique<ValuesExecutor>(
            emptyRows,
            schema
        ),
        std::move(keys)
    );

    executor.Init();

    Row result;

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ============================================================
// 10. Single row
// ============================================================

TEST_F(SortExecutorTest, SingleRow)
{
    auto singleRow =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{10},
                    std::string{"Alice"},
                    int64_t{95}
                })}
            }
        );

    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(2, SortDirection::DESC)
    );

    SortExecutor executor(
        std::make_unique<ValuesExecutor>(
            singleRow,
            schema
        ),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 1);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{10})
    );

    executor.Close();
}

// ============================================================
// 11. Duplicate rows are preserved
// ============================================================

TEST_F(SortExecutorTest, PreservesDuplicates)
{
    auto duplicateRows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{2},
                    std::string{"Bob"},
                    int64_t{75}
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

    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        std::make_unique<ValuesExecutor>(
            duplicateRows,
            schema
        ),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{1, 2, 2})
    );

    EXPECT_EQ(
        results[1].tuple.getTupleValues(),
        results[2].tuple.getTupleValues()
    );

    executor.Close();
}

// ============================================================
// 12. Output schema matches child schema
// ============================================================

TEST_F(SortExecutorTest, OutputSchemaMatchesChild)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    const QuerySchema& outputSchema =
        executor.OutputSchema();

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
// 13. Init resets sorting state
// ============================================================

TEST_F(SortExecutorTest, InitResetsIteration)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto firstResults = CollectRows(executor);

    executor.Close();
    executor.Init();

    auto secondResults = CollectRows(executor);

    EXPECT_EQ(
        GetIDs(firstResults),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );

    EXPECT_EQ(
        GetIDs(secondResults),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );

    executor.Close();
}

// ============================================================
// 14. Close before exhaustion
// ============================================================

TEST_F(SortExecutorTest, CloseBeforeExhaustion)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    executor.Close();
    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{1, 2, 3, 4, 5})
    );

    executor.Close();
}

// ============================================================
// 15. Close is idempotent
// ============================================================

TEST_F(SortExecutorTest, CloseIsIdempotent)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 5);

    executor.Close();
}

// ============================================================
// 16. Exhaustion remains exhausted
// ============================================================

TEST_F(SortExecutorTest, ExhaustionReturnsFalse)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
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
// 17. Returned rows remain valid after Close
// ============================================================

TEST_F(SortExecutorTest, ReturnedRowsRemainValid)
{
    std::vector<SortKey> keys;

    keys.push_back(
        ColumnKey(0, SortDirection::ASC)
    );

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
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
// 18. Sort by computed expression
// ORDER BY score + id ASC
// ============================================================

TEST_F(SortExecutorTest, ComputedSortExpression)
{
    std::vector<SortKey> keys;

    keys.push_back({
        std::make_unique<AdditionExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ColumnExpression>(0)
        ),
        SortDirection::ASC
    });

    SortExecutor executor(
        MakeValuesExecutor(),
        std::move(keys)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    // score + id:
    // Carol = 88
    // Eve   = 95
    // Bob   = 77
    // Alice = 91
    // Dan   = 79

    EXPECT_EQ(
        GetIDs(results),
        (std::vector<int64_t>{2, 4, 3, 1, 5})
    );

    executor.Close();
}
