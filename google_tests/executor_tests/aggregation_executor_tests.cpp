
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <algorithm>

#include "aggregation_executor.hpp"
#include "values_executor.hpp"

#include "column_expression.hpp"
#include "constant_expression.hpp"
#include "addition_expression.hpp"

// ============================================================
// Test Fixture
// ============================================================

class AggregationExecutorTest : public ::testing::Test
{
protected:
    std::shared_ptr<const QuerySchema> schema;
    std::shared_ptr<const std::vector<Row>> rows;

    void SetUp() override
    {
        // Input schema:
        // id INT64, department STRING32, score INT64, year INT64

        schema = std::make_shared<const QuerySchema>(
            std::vector<Field>{
                Field{"id", FieldType::INT64},
                Field{"department", FieldType::STRING32},
                Field{"score", FieldType::INT64},
                Field{"year", FieldType::INT64}
            }
        );

        rows = std::make_shared<const std::vector<Row>>(
            std::vector<Row>{
                Row{Tuple({
                    int64_t{1}, std::string{"CS"},
                    int64_t{90}, int64_t{2025}
                })},
                Row{Tuple({
                    int64_t{2}, std::string{"Math"},
                    int64_t{75}, int64_t{2025}
                })},
                Row{Tuple({
                    int64_t{3}, std::string{"CS"},
                    int64_t{85}, int64_t{2025}
                })},
                Row{Tuple({
                    int64_t{4}, std::string{"Math"},
                    int64_t{95}, int64_t{2026}
                })},
                Row{Tuple({
                    int64_t{5}, std::string{"Physics"},
                    int64_t{80}, int64_t{2026}
                })},
                Row{Tuple({
                    int64_t{6}, std::string{"CS"},
                    int64_t{70}, int64_t{2026}
                })}
            }
        );
    }

    std::unique_ptr<Executor> MakeValuesExecutor()
    {
        return std::make_unique<ValuesExecutor>(
            rows, schema
        );
    }

    std::unique_ptr<Executor> MakeValuesExecutor(
        std::vector<Row> input
    )
    {
        return std::make_unique<ValuesExecutor>(
            std::make_shared<const std::vector<Row>>(
                std::move(input)
            ),
            schema
        );
    }

    static GroupByExpression GroupColumn(
        std::string name,
        FieldType type,
        std::size_t index
    )
    {
        return GroupByExpression{
            std::make_unique<ColumnExpression>(index),
            Field{std::move(name), type}
        };
    }

    static AggregationExpression Count()
    {
        return AggregationExpression{
            Field{"count", FieldType::INT64},
            AggregationType::COUNT,
            nullptr
        };
    }

    static AggregationExpression Sum(std::size_t index)
    {
        return AggregationExpression{
            Field{"sum", FieldType::INT64},
            AggregationType::SUM,
            std::make_unique<ColumnExpression>(index)
        };
    }

    static AggregationExpression Min(std::size_t index)
    {
        return AggregationExpression{
            Field{"min", FieldType::INT64},
            AggregationType::MIN,
            std::make_unique<ColumnExpression>(index)
        };
    }

    static AggregationExpression Max(std::size_t index)
    {
        return AggregationExpression{
            Field{"max", FieldType::INT64},
            AggregationType::MAX,
            std::make_unique<ColumnExpression>(index),
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
        EXPECT_EQ(row.tuple.getTupleValues(), expected);
    }

    static const Row* FindGroup(
        const std::vector<Row>& results,
        const std::string& department
    )
    {
        for (const Row& row : results)
        {
            const auto& values = row.tuple.getTupleValues();

            if (std::get<std::string>(values.at(0)) ==
                department)
            {
                return &row;
            }
        }

        return nullptr;
    }

    static std::vector<GroupByExpression> DepartmentGrouping()
    {
        std::vector<GroupByExpression> groups;
        groups.push_back(
            GroupColumn("department", FieldType::STRING32, 1)
        );
        return groups;
    }

    static std::vector<AggregationExpression> AllAggregates()
    {
        std::vector<AggregationExpression> aggregates;
        aggregates.push_back(Count());
        aggregates.push_back(Sum(2));
        aggregates.push_back(Min(2));
        aggregates.push_back(Max(2));
        return aggregates;
    }
};

// ============================================================
// 1. Global COUNT(*)
// ============================================================

TEST_F(AggregationExecutorTest, GlobalCount)
{
    std::vector<GroupByExpression> groups;
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        std::move(groups),
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 1);
    ExpectValues(results[0], {int64_t{6}});
    EXPECT_FALSE(results[0].source.has_value());

    executor.Close();
}

// ============================================================
// 2. Global SUM
// Scores: 90 + 75 + 85 + 95 + 80 + 70 = 495
// ============================================================

TEST_F(AggregationExecutorTest, GlobalSum)
{
    std::vector<GroupByExpression> groups;
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Sum(2));

    AggregationExecutor executor(
        MakeValuesExecutor(),
        std::move(groups),
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 1);
    ExpectValues(results[0], {int64_t{495}});

    executor.Close();
}

// ============================================================
// 3. Global MIN and MAX
// ============================================================

TEST_F(AggregationExecutorTest, GlobalMinMax)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Min(2));
    aggregates.push_back(Max(2));

    AggregationExecutor executor(
        MakeValuesExecutor(),
        {},
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 1);

    ExpectValues(results[0], {
        int64_t{70},
        int64_t{95}
    });

    executor.Close();
}

// ============================================================
// 4. Group by department with COUNT(*)
// CS = 3, Math = 2, Physics = 1
// ============================================================

TEST_F(AggregationExecutorTest, GroupByCount)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    const Row* cs = FindGroup(results, "CS");
    const Row* math = FindGroup(results, "Math");
    const Row* physics = FindGroup(results, "Physics");

    ASSERT_NE(cs, nullptr);
    ASSERT_NE(math, nullptr);
    ASSERT_NE(physics, nullptr);

    ExpectValues(*cs, {std::string{"CS"}, int64_t{3}});
    ExpectValues(*math, {std::string{"Math"}, int64_t{2}});
    ExpectValues(*physics, {std::string{"Physics"}, int64_t{1}});

    executor.Close();
}

// ============================================================
// 5. Group by department with SUM
// CS = 245, Math = 170, Physics = 80
// ============================================================

TEST_F(AggregationExecutorTest, GroupBySum)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Sum(2));

    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    const Row* cs = FindGroup(results, "CS");
    const Row* math = FindGroup(results, "Math");
    const Row* physics = FindGroup(results, "Physics");

    ASSERT_NE(cs, nullptr);
    ASSERT_NE(math, nullptr);
    ASSERT_NE(physics, nullptr);

    ExpectValues(*cs, {std::string{"CS"}, int64_t{245}});
    ExpectValues(*math, {std::string{"Math"}, int64_t{170}});
    ExpectValues(*physics, {std::string{"Physics"}, int64_t{80}});

    executor.Close();
}

// ============================================================
// 6. Group by department with COUNT, SUM, MIN, MAX
// ============================================================

TEST_F(AggregationExecutorTest, MultipleAggregatesPerGroup)
{
    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        AllAggregates()
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    const Row* cs = FindGroup(results, "CS");
    const Row* math = FindGroup(results, "Math");
    const Row* physics = FindGroup(results, "Physics");

    ASSERT_NE(cs, nullptr);
    ASSERT_NE(math, nullptr);
    ASSERT_NE(physics, nullptr);

    ExpectValues(*cs, {
        std::string{"CS"},
        int64_t{3},
        int64_t{245},
        int64_t{70},
        int64_t{90}
    });

    ExpectValues(*math, {
        std::string{"Math"},
        int64_t{2},
        int64_t{170},
        int64_t{75},
        int64_t{95}
    });

    ExpectValues(*physics, {
        std::string{"Physics"},
        int64_t{1},
        int64_t{80},
        int64_t{80},
        int64_t{80}
    });

    executor.Close();
}

// ============================================================
// 7. Group by multiple columns: department and year
// ============================================================

TEST_F(AggregationExecutorTest, MultipleGroupingColumns)
{
    std::vector<GroupByExpression> groups;
    groups.push_back(
        GroupColumn("department", FieldType::STRING32, 1)
    );
    groups.push_back(
        GroupColumn("year", FieldType::INT64, 3)
    );

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        std::move(groups),
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 5);

    std::vector<std::vector<FieldVariant>> actual;

    for (const Row& row : results)
    {
        actual.push_back(row.tuple.getTupleValues());
    }

    std::sort(actual.begin(), actual.end());

    std::vector<std::vector<FieldVariant>> expected = {
        {std::string{"CS"}, int64_t{2025}, int64_t{2}},
        {std::string{"CS"}, int64_t{2026}, int64_t{1}},
        {std::string{"Math"}, int64_t{2025}, int64_t{1}},
        {std::string{"Math"}, int64_t{2026}, int64_t{1}},
        {std::string{"Physics"}, int64_t{2026}, int64_t{1}}
    };

    EXPECT_EQ(actual, expected);

    executor.Close();
}

// ============================================================
// 8. Computed aggregate expression: SUM(score + 5)
// Expected: 495 + (6 * 5) = 525
// ============================================================

TEST_F(AggregationExecutorTest, ComputedAggregateExpression)
{
    std::vector<AggregationExpression> aggregates;

    aggregates.push_back({
        Field{"adjusted_sum", FieldType::INT64},
        AggregationType::SUM,
        std::make_unique<AdditionExpression>(
            std::make_unique<ColumnExpression>(2),
            std::make_unique<ConstantExpression>(int64_t{5})
        ),
    });

    AggregationExecutor executor(
        MakeValuesExecutor(),
        {},
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 1);
    ExpectValues(results[0], {int64_t{525}});

    executor.Close();
}

// ============================================================
// 9. Empty input with GROUP BY returns zero rows
// ============================================================

TEST_F(AggregationExecutorTest, EmptyGroupedInput)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor({}),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();

    const auto& fields = executor.OutputSchema().getFieldList();

    ASSERT_EQ(fields.size(), 2);

    EXPECT_EQ(fields[0].name, "department");
    EXPECT_EQ(fields[0].type, FieldType::STRING32);

    EXPECT_EQ(fields[1].name, "count");
    EXPECT_EQ(fields[1].type, FieldType::INT64);

    executor.Close();
}

// ============================================================
// 10. Empty global COUNT(*) returns one row with zero
// ============================================================

TEST_F(AggregationExecutorTest, EmptyGlobalCount)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor({}),
        {},
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    const auto& fields = executor.OutputSchema().getFieldList();
    ASSERT_EQ(fields.size(), 1);
    EXPECT_EQ(fields[0].name, "count");
    EXPECT_EQ(fields[0].type, FieldType::INT64);
    executor.Close();
}

// ============================================================
// 11. SUM handles negative integers
// ============================================================

TEST_F(AggregationExecutorTest, SumNegativeValues)
{
    std::vector<Row> input = {
        Row{Tuple({
            int64_t{1}, std::string{"CS"},
            int64_t{-10}, int64_t{2025}
        })},
        Row{Tuple({
            int64_t{2}, std::string{"CS"},
            int64_t{5}, int64_t{2025}
        })},
        Row{Tuple({
            int64_t{3}, std::string{"CS"},
            int64_t{-20}, int64_t{2025}
        })}
    };

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Sum(2));

    AggregationExecutor executor(
        MakeValuesExecutor(std::move(input)),
        {},
        std::move(aggregates)
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 1);
    ExpectValues(results[0], {int64_t{-25}});

    executor.Close();
}

// ============================================================
// 12. SUM detects positive overflow
// ============================================================

TEST_F(AggregationExecutorTest, SumPositiveOverflow)
{
    std::vector<Row> input = {
        Row{Tuple({
            int64_t{1}, std::string{"CS"},
            std::numeric_limits<int64_t>::max(), int64_t{2025}
        })},
        Row{Tuple({
            int64_t{2}, std::string{"CS"},
            int64_t{1}, int64_t{2025}
        })}
    };

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Sum(2));

    AggregationExecutor executor(
        MakeValuesExecutor(std::move(input)),
        {},
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::overflow_error);

    executor.Close();
}

// ============================================================
// 13. SUM detects negative overflow
// ============================================================

TEST_F(AggregationExecutorTest, SumNegativeOverflow)
{
    std::vector<Row> input = {
        Row{Tuple({
            int64_t{1}, std::string{"CS"},
            std::numeric_limits<int64_t>::min(), int64_t{2025}
        })},
        Row{Tuple({
            int64_t{2}, std::string{"CS"},
            int64_t{-1}, int64_t{2025}
        })}
    };

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Sum(2));

    AggregationExecutor executor(
        MakeValuesExecutor(std::move(input)),
        {},
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::overflow_error);

    executor.Close();
}

// ============================================================
// 14. Invalid grouping expression type
// ============================================================

TEST_F(AggregationExecutorTest, RejectsGroupingTypeMismatch)
{
    std::vector<GroupByExpression> groups;

    // Department is STRING32, not INT64.
    groups.push_back(
        GroupColumn("department", FieldType::INT64, 1)
    );

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        std::move(groups),
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::invalid_argument);

    executor.Close();
}

// ============================================================
// 15. Boolean grouping expression is rejected
// ============================================================

TEST_F(AggregationExecutorTest, RejectsBooleanGrouping)
{
    std::vector<GroupByExpression> groups;

    groups.push_back({
        std::make_unique<ConstantExpression>(true),
        Field{"invalid", FieldType::INT64}
    });

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        std::move(groups),
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::invalid_argument);

    executor.Close();
}

// ============================================================
// 16. SUM rejects string expressions
// ============================================================

TEST_F(AggregationExecutorTest, RejectsStringSum)
{
    std::vector<AggregationExpression> aggregates;

    aggregates.push_back({
        Field{"invalid_sum", FieldType::INT64},
        AggregationType::SUM,
        std::make_unique<ColumnExpression>(1)
    });

    AggregationExecutor executor(
        MakeValuesExecutor(),
        {},
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::invalid_argument);

    executor.Close();
}

// ============================================================
// 17. MIN rejects string expressions in current implementation
// ============================================================

TEST_F(AggregationExecutorTest, RejectsStringMin)
{
    std::vector<AggregationExpression> aggregates;

    aggregates.push_back({
        Field{"invalid_min", FieldType::STRING32},
        AggregationType::MIN,
        std::make_unique<ColumnExpression>(1),
    });

    AggregationExecutor executor(
        MakeValuesExecutor(),
        {},
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    EXPECT_THROW(executor.Next(result), std::invalid_argument);

    executor.Close();
}

// ============================================================
// 18. Output schema matches grouping and aggregate columns
// ============================================================

TEST_F(AggregationExecutorTest, OutputSchema)
{
    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        AllAggregates()
    );

    const auto& fields =
        executor.OutputSchema().getFieldList();

    ASSERT_EQ(fields.size(), 5);

    EXPECT_EQ(fields[0].name, "department");
    EXPECT_EQ(fields[0].type, FieldType::STRING32);

    EXPECT_EQ(fields[1].name, "count");
    EXPECT_EQ(fields[1].type, FieldType::INT64);

    EXPECT_EQ(fields[2].name, "sum");
    EXPECT_EQ(fields[2].type, FieldType::INT64);

    EXPECT_EQ(fields[3].name, "min");
    EXPECT_EQ(fields[3].type, FieldType::INT64);

    EXPECT_EQ(fields[4].name, "max");
    EXPECT_EQ(fields[4].type, FieldType::INT64);
}

// ============================================================
// 19. Reinitialization resets aggregation
// ============================================================

TEST_F(AggregationExecutorTest, InitResetsAggregation)
{
    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        AllAggregates()
    );

    executor.Init();

    auto firstResults = CollectRows(executor);

    executor.Close();
    executor.Init();

    auto secondResults = CollectRows(executor);

    ASSERT_EQ(firstResults.size(), 3);
    ASSERT_EQ(secondResults.size(), 3);

    auto normalize = [](const std::vector<Row>& input)
    {
        std::vector<std::vector<FieldVariant>> values;

        for (const Row& row : input)
            values.push_back(row.tuple.getTupleValues());

        std::sort(values.begin(), values.end());
        return values;
    };

    EXPECT_EQ(
        normalize(firstResults),
        normalize(secondResults)
    );

    executor.Close();
}

// ============================================================
// 20. Exhaustion remains exhausted
// ============================================================

TEST_F(AggregationExecutorTest, ExhaustionReturnsFalse)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    for (int i = 0; i < 3; ++i)
        ASSERT_TRUE(executor.Next(result));

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}

// ============================================================
// 21. Close before exhaustion allows reinitialization
// ============================================================

TEST_F(AggregationExecutorTest, CloseBeforeExhaustion)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    executor.Close();
    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 3);

    executor.Close();
}

// ============================================================
// 22. Close is idempotent
// ============================================================

TEST_F(AggregationExecutorTest, CloseIsIdempotent)
{
    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());

    executor.Init();

    auto results = CollectRows(executor);

    EXPECT_EQ(results.size(), 3);

    EXPECT_NO_THROW(executor.Close());
    EXPECT_NO_THROW(executor.Close());
}

// ============================================================
// 23. Output rows have no source references
// ============================================================

TEST_F(AggregationExecutorTest, OutputRowsHaveNoSource)
{
    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        AllAggregates()
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    for (const Row& row : results)
        EXPECT_FALSE(row.source.has_value());

    executor.Close();
}

// ============================================================
// 24. Returned rows remain valid after Close
// ============================================================

TEST_F(AggregationExecutorTest, ReturnedRowsRemainValid)
{
    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        AllAggregates()
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


TEST_F(AggregationExecutorTest, GroupByWithoutAggregates)
{
    AggregationExecutor executor(
        MakeValuesExecutor(),
        DepartmentGrouping(),
        {}
    );

    executor.Init();

    auto results = CollectRows(executor);

    ASSERT_EQ(results.size(), 3);

    std::vector<std::string> departments;

    for (const Row& row : results)
    {
        const auto& values = row.tuple.getTupleValues();

        ASSERT_EQ(values.size(), 1);

        departments.push_back(
            std::get<std::string>(values[0])
        );
    }

    std::sort(departments.begin(), departments.end());

    EXPECT_EQ(
        departments,
        (std::vector<std::string>{"CS", "Math", "Physics"})
    );

    executor.Close();
}


class CountingAggregationChild : public Executor
{
private:
    std::unique_ptr<Executor> child;
    std::shared_ptr<std::size_t> nextCalls;

public:
    CountingAggregationChild(
        std::unique_ptr<Executor> child,
        std::shared_ptr<std::size_t> counter
    )
        : child(std::move(child)),
          nextCalls(std::move(counter))
    {}

    void Init() override
    {
        child->Init();
        *nextCalls = 0;
    }

    bool Next(Row& out) override
    {
        ++(*nextCalls);
        return child->Next(out);
    }

    void Close() override
    {
        child->Close();
    }

    const QuerySchema& OutputSchema() const override
    {
        return child->OutputSchema();
    }
};

TEST_F(AggregationExecutorTest, BlockingBehavior)
{
    auto counter = std::make_shared<std::size_t>(0);

    auto child = std::make_unique<CountingAggregationChild>(
        MakeValuesExecutor(),
        counter
    );

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());

    AggregationExecutor executor(
        std::move(child),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    executor.Init();

    EXPECT_EQ(*counter, 0);

    Row result;

    ASSERT_TRUE(executor.Next(result));

    // Six input rows and one end-of-input check.
    EXPECT_EQ(*counter, 7);

    ASSERT_TRUE(executor.Next(result));

    // Subsequent calls must not read the child again.
    EXPECT_EQ(*counter, 7);

    executor.Close();
}
TEST_F(AggregationExecutorTest, EmptyGlobalCountWithOtherAggregates)
{
    AggregationExecutor executor(
        MakeValuesExecutor({}),
        {},
        AllAggregates()
    );

    executor.Init();

    Row result;

    EXPECT_FALSE(executor.Next(result));
    EXPECT_FALSE(executor.Next(result));

    const auto& fields = executor.OutputSchema().getFieldList();

    ASSERT_EQ(fields.size(), 4);

    EXPECT_EQ(fields[0].name, "count");
    EXPECT_EQ(fields[1].name, "sum");
    EXPECT_EQ(fields[2].name, "min");
    EXPECT_EQ(fields[3].name, "max");

    for (const Field& field : fields)
    {
        EXPECT_EQ(field.type, FieldType::INT64);
    }

    executor.Close();
}

TEST_F(AggregationExecutorTest, CountIncludesDuplicateRows)
{
    std::vector<Row> input = {
        Row{Tuple({
            int64_t{1},
            std::string{"CS"},
            int64_t{90},
            int64_t{2025}
        })},
        Row{Tuple({
            int64_t{1},
            std::string{"CS"},
            int64_t{90},
            int64_t{2025}
        })},
        Row{Tuple({
            int64_t{1},
            std::string{"CS"},
            int64_t{90},
            int64_t{2025}
        })}
    };

    std::vector<AggregationExpression> aggregates;
    aggregates.push_back(Count());
    aggregates.push_back(Sum(2));

    AggregationExecutor executor(
        MakeValuesExecutor(std::move(input)),
        DepartmentGrouping(),
        std::move(aggregates)
    );

    executor.Init();

    Row result;

    ASSERT_TRUE(executor.Next(result));

    ExpectValues(result, {
        std::string{"CS"},
        int64_t{3},
        int64_t{270}
    });

    EXPECT_FALSE(executor.Next(result));

    executor.Close();
}