#include "nested_loop_join_executor.hpp"

NestedLoopExecutor::NestedLoopExecutor(std::unique_ptr<Executor> leftChild, std::unique_ptr<Executor> rightChild, std::unique_ptr<Expression> pred) : 
    leftChildExecutor(std::move(leftChild)), rightChildExecutor(std::move(rightChild)), joinPredicate(std::move(pred))
{
    std::vector<Field> schemaField;

    std::vector<Field> leftSchemaField = leftChildExecutor->OutputSchema().getFieldList();
    std::vector<Field> rightSchemaField = rightChildExecutor->OutputSchema().getFieldList();

    for(const Field field : leftSchemaField)
    {
        schemaField.push_back(field);
    }
    for(const Field field : rightSchemaField)
    {
        schemaField.push_back(field);
    }

    schemaPointer = std::make_shared<const QuerySchema>(schemaField);

}

void NestedLoopExecutor::Init()
{
    leftChildExecutor->Init();
    rightChildExecutor->Init();
    // Materialize right child into vector of owned rows once
    Row rightRow;
    while(rightChildExecutor->Next(rightRow))
    {
        rightChildRows.push_back(rightRow);
    }
    rightChildExecutor->Init();
    leftRow.tuple = Tuple({});

}

bool NestedLoopExecutor::Next(Row& out)
{
    if(rightChildRows.empty()) return false;

    while(true)
    {
        if(leftRow.tuple.getTupleValues().empty())
        {
            Row leftChildRow;
            if(!leftChildExecutor->Next(leftChildRow)) return false; // out of left rows
            leftRow = std::move(leftChildRow);
        }

        if(rightRowIndex >= rightChildRows.size()) // compared all right rows, reset
        {
            leftRow.tuple = Tuple({});
            rightRowIndex = 0;
            continue;
        }

        std::vector<FieldVariant> leftRowValues = leftRow.tuple.getTupleValues();
        std::vector<FieldVariant> joinedValues;
        for(FieldVariant& val : leftRowValues)
        {
            joinedValues.push_back(val);
        }

        std::vector<FieldVariant> rightRowValues = rightChildRows[rightRowIndex++].tuple.getTupleValues();
        for(FieldVariant& val : rightRowValues)
        {
            joinedValues.push_back(val);
        }

        Row combinedRow{Tuple(std::move(joinedValues)), std::nullopt};
        ExpressionVariant joinResult = joinPredicate->Eval(combinedRow);
        if(!std::holds_alternative<bool>(joinResult))
        {
            throw std::invalid_argument("[Nested Loop Join - Next]: Join Predicate should return bool");
        }

        if(!std::get<bool>(joinResult)) continue;

        out = std::move(combinedRow);
        return true;
    }

    return false;
}