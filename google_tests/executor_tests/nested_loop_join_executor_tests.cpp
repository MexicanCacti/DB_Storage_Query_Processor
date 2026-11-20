
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "nested_loop_join_executor.hpp"
#include "values_executor.hpp"

#include "constant_expression.hpp"
#include "column_expression.hpp"
#include "equal_expression.hpp"
#include "greater_than_expression.hpp"

// ============================================================
// Test Fixture
// ============================================================

class NestedLoopExecutorTest : public ::testing::Test
{
protected:
    std::shared_ptr<const QuerySchema> leftSchema;
    std::shared_ptr<const QuerySchema> rightSchema;

    std::shared_ptr<const std::vector<Row>> leftRows;
    std::shared_ptr<const std::vector<Row>> rightRows;

    void SetUp() override
    {
        // Left table:
        // students(id INT64, name STRING32, dept STRING32)

        leftSchema = std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64},
                Field{"name", FieldType::STRING32},
                Field{"dept", FieldType::STRING32}
            }
        );

        // Right table:
        // departments(dept STRING32, building STRING32)

        rightSchema = std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"dept", FieldType::STRING32},
                Field{"building", FieldType::STRING32}
            }
        );

        leftRows = std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    std::string{"CS"}
                })},
                Row{Tuple({
                    int64_t{2},
                    std::string{"Bob"},
                    std::string{"CS"}
                })},
                Row{Tuple({
                    int64_t{3},
                    std::string{"Carol"},
                    std::string{"Math"}
                })},
                Row{Tuple({
                    int64_t{4},
                    std::string{"Dan"},
                    std::string{"Math"}
                })},
                Row{Tuple({
                    int64_t{5},
                    std::string{"Eve"},
                    std::string{"CS"}
                })}
            }
        );

        rightRows = std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    std::string{"CS"},
                    std::string{"Tech"}
                })},
                Row{Tuple({
                    std::string{"Math"},
                    std::string{"Science"}
                })},
                Row{Tuple({
                    std::string{"Art"},
                    std::string{"Studio"}
                })}
            }
        );
    }

    std::unique_ptr<Executor> MakeLeftExecutor()
    {
        return std::make_unique<ValuesExecutor>(
            leftRows,
            leftSchema
        );
    }

    std::unique_ptr<Executor> MakeRightExecutor()
    {
        return std::make_unique<ValuesExecutor>(
            rightRows,
            rightSchema
        );
    }

    // The joined row has five columns:
    //
    // 0: students.id
    // 1: students.name
    // 2: students.dept
    // 3: departments.dept
    // 4: departments.building

    static std::unique_ptr<Expression> MakeDeptPredicate()
    {
        return std::make_unique<EqualExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ColumnExpression>(3)
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
        const QuerySchema& schema,
        const std::vector<Field>& expected
    )
    {
        const auto& actual = schema.getFieldList();

        ASSERT_EQ(actual.size(), expected.size());

        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            EXPECT_EQ(actual[i].name, expected[i].name);
            EXPECT_EQ(actual[i].type, expected[i].type);
        }
    }

    static int64_t GetID(const Row& row)
    {
        return std::get<int64_t>(
            row.tuple.getTupleValues().at(0)
        );
    }

    static std::string GetBuilding(const Row& row)
    {
        return std::get<std::string>(
            row.tuple.getTupleValues().at(4)
        );
    }
};

// ============================================================
// 1. Basic inner join
//
// students.dept == departments.dept
// Expected: five matching rows
// ============================================================

TEST_F(NestedLoopExecutorTest, BasicInnerJoin)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    std::sort(
        results.begin(),
        results.end(),
        [](const Row& a, const Row& b)
        {
            return GetID(a) < GetID(b);
        }
    );

    ExpectValues(
        results[0],
        {
            int64_t{1},
            std::string{"Alice"},
            std::string{"CS"},
            std::string{"CS"},
            std::string{"Tech"}
        }
    );

    ExpectValues(
        results[1],
        {
            int64_t{2},
            std::string{"Bob"},
            std::string{"CS"},
            std::string{"CS"},
            std::string{"Tech"}
        }
    );

    ExpectValues(
        results[2],
        {
            int64_t{3},
            std::string{"Carol"},
            std::string{"Math"},
            std::string{"Math"},
            std::string{"Science"}
        }
    );

    ExpectValues(
        results[3],
        {
            int64_t{4},
            std::string{"Dan"},
            std::string{"Math"},
            std::string{"Math"},
            std::string{"Science"}
        }
    );

    ExpectValues(
        results[4],
        {
            int64_t{5},
            std::string{"Eve"},
            std::string{"CS"},
            std::string{"CS"},
            std::string{"Tech"}
        }
    );

    executor.Close();
}

// ============================================================
// 2. Output schema combines left and right columns
// ============================================================

TEST_F(NestedLoopExecutorTest, OutputSchemaCombinesChildren)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
    );

    ExpectSchema(
        executor.OutputSchema(),
        {
            Field{"id", FieldType::INT64},
            Field{"name", FieldType::STRING32},
            Field{"dept", FieldType::STRING32},
            Field{"dept", FieldType::STRING32},
            Field{"building", FieldType::STRING32}
        }
    );
}

// ============================================================
// 3. Nonmatching rows are excluded
// ============================================================

TEST_F(NestedLoopExecutorTest, NonMatchingRowsExcluded)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    for (const auto& row : results)
    {
        const auto& values = row.tuple.getTupleValues();

        EXPECT_EQ(
            std::get<std::string>(values[2]),
            std::get<std::string>(values[3])
        );

        EXPECT_NE(GetBuilding(row), "Studio");
    }

    executor.Close();
}

// ============================================================
// 4. No matching pairs
// ============================================================

TEST_F(NestedLoopExecutorTest, NoMatchingPairs)
{
    auto unmatchedRows =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    std::string{"Physics"},
                    std::string{"Building A"}
                })}
            }
        );

    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        std::make_unique<ValuesExecutor>(
            unmatchedRows,
            rightSchema
        ),
        MakeDeptPredicate()
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 5. Empty left child
// ============================================================

TEST_F(NestedLoopExecutorTest, EmptyLeftChild)
{
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    NestedLoopExecutor executor(
        std::make_unique<ValuesExecutor>(
            emptyRows,
            leftSchema
        ),
        MakeRightExecutor(),
        MakeDeptPredicate()
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 6. Empty right child
// ============================================================

TEST_F(NestedLoopExecutorTest, EmptyRightChild)
{
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        std::make_unique<ValuesExecutor>(
            emptyRows,
            rightSchema
        ),
        MakeDeptPredicate()
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));
    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 7. Both children empty
// ============================================================

TEST_F(NestedLoopExecutorTest, BothChildrenEmpty)
{
    auto emptyRows =
        std::make_shared<const std::vector<Row>>();

    NestedLoopExecutor executor(
        std::make_unique<ValuesExecutor>(
            emptyRows,
            leftSchema
        ),
        std::make_unique<ValuesExecutor>(
            emptyRows,
            rightSchema
        ),
        MakeDeptPredicate()
    );

    executor.Init();

    Row row;

    EXPECT_FALSE(executor.Next(row));

    executor.Close();
}

// ============================================================
// 8. Duplicate keys produce every matching pair
//
// Two left CS rows x three right CS rows = six results.
// ============================================================

TEST_F(NestedLoopExecutorTest, DuplicateKeysProduceAllPairs)
{
    auto duplicateLeft =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    std::string{"CS"}
                })},
                Row{Tuple({
                    int64_t{2},
                    std::string{"Bob"},
                    std::string{"CS"}
                })}
            }
        );

    auto duplicateRight =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    std::string{"CS"},
                    std::string{"Tech1"}
                })},
                Row{Tuple({
                    std::string{"CS"},
                    std::string{"Tech2"}
                })},
                Row{Tuple({
                    std::string{"CS"},
                    std::string{"Tech3"}
                })}
            }
        );

    NestedLoopExecutor executor(
        std::make_unique<ValuesExecutor>(
            duplicateLeft,
            leftSchema
        ),
        std::make_unique<ValuesExecutor>(
            duplicateRight,
            rightSchema
        ),
        MakeDeptPredicate()
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 6);

    std::vector<std::pair<int64_t, std::string>> actual;

    for (const auto& row : results)
    {
        actual.emplace_back(
            GetID(row),
            GetBuilding(row)
        );
    }

    std::sort(actual.begin(), actual.end());

    std::vector<std::pair<int64_t, std::string>> expected = {
        {1, "Tech1"},
        {1, "Tech2"},
        {1, "Tech3"},
        {2, "Tech1"},
        {2, "Tech2"},
        {2, "Tech3"}
    };

    EXPECT_EQ(actual, expected);

    executor.Close();
}

// ============================================================
// 9. Duplicate identical rows are preserved
// ============================================================

TEST_F(NestedLoopExecutorTest, PreservesIdenticalDuplicates)
{
    auto duplicateLeft =
        std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    std::string{"CS"}
                })},
                Row{Tuple({
                    int64_t{1},
                    std::string{"Alice"},
                    std::string{"CS"}
                })}
            }
        );

    NestedLoopExecutor executor(
        std::make_unique<ValuesExecutor>(
            duplicateLeft,
            leftSchema
        ),
        MakeRightExecutor(),
        MakeDeptPredicate()
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
// 10. Integer comparison predicate
//
// Join where students.id > 3
// Every qualifying left row pairs with every right row.
// ============================================================

TEST_F(NestedLoopExecutorTest, IntegerPredicate)
{
    auto predicate =
        std::make_unique<GreaterThanExpression>(
            std::make_unique<ColumnExpression>(0),
            std::make_unique<ConstantExpression>(
                int64_t{3}
            )
        );

    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        std::move(predicate)
    );

    executor.Init();

    auto results = CollectRows(executor);

    // Dan and Eve each pair with all three departments.
    EXPECT_EQ(results.size(), 6);

    for (const auto& row : results)
    {
        EXPECT_GT(GetID(row), 3);
    }

    executor.Close();
}

// ============================================================
// 11. Source references are removed
// ============================================================

TEST_F(NestedLoopExecutorTest, RemovesSourceReferences)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
    );

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));

    EXPECT_FALSE(row.source.has_value());

    executor.Close();
}

// ============================================================
// 12. Init resets iteration
// ============================================================

TEST_F(NestedLoopExecutorTest, InitResetsIteration)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
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
// 13. Close before exhaustion
// ============================================================

TEST_F(NestedLoopExecutorTest, CloseBeforeExhaustion)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
    );

    executor.Init();

    Row row;

    ASSERT_TRUE(executor.Next(row));

    executor.Close();

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 5);

    executor.Close();
}

// ============================================================
// 14. Close is idempotent
// ============================================================

TEST_F(NestedLoopExecutorTest, CloseIsIdempotent)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
    );

    EXPECT_NO_THROW(executor.Close());
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
// 15. Exhaustion remains exhausted
// ============================================================

TEST_F(NestedLoopExecutorTest, ExhaustionReturnsFalse)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
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
// 16. Returned rows remain valid after later Next calls
// ============================================================

TEST_F(NestedLoopExecutorTest, ReturnedRowsRemainValid)
{
    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
        MakeDeptPredicate()
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
// 17. Non-Boolean join predicate throws
// ============================================================

TEST_F(NestedLoopExecutorTest, RejectsNonBooleanPredicate)
{
    auto predicate =
        std::make_unique<ConstantExpression>(
            int64_t{10}
        );

    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
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
// 18. Invalid column index throws
// ============================================================

TEST_F(NestedLoopExecutorTest, RejectsInvalidColumnIndex)
{
    auto predicate =
        std::make_unique<EqualExpression>(
            std::make_unique<ColumnExpression>(100),
            std::make_unique<ConstantExpression>(
                int64_t{1}
            )
        );

    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
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
// 19. Incompatible comparison types throw
// ============================================================

TEST_F(NestedLoopExecutorTest, RejectsIncompatibleTypes)
{
    auto predicate =
        std::make_unique<EqualExpression>(
            std::make_unique<ColumnExpression>(0),
            std::make_unique<ColumnExpression>(3)
        );

    NestedLoopExecutor executor(
        MakeLeftExecutor(),
        MakeRightExecutor(),
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
