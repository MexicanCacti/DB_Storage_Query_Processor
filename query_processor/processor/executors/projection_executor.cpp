#include "projection_executor.hpp"

ProjectionExecutor::ProjectionExecutor(std::unique_ptr<Executor> child, std::vector<ProjectionColumn> pList) :
    childExecutor(std::move(child)), projectionList(std::move(pList))
{
    std::vector<Field> schemaValues;
    for(ProjectionColumn& projection : projectionList)
    {
        schemaValues.push_back({projection.projectionField.name, projection.projectionField.type});
    }

    projectionSchema = QuerySchema(schemaValues);
}

bool ProjectionExecutor::Next(Row& out)
{
    if(!childExecutor->Next(out)) return false;
    std::vector<FieldVariant> projectionValues;

    for(ProjectionColumn& projection : projectionList)
    {
        ExpressionVariant projectionResult = projection.projectionExpression->Eval(out);
        if(std::holds_alternative<bool>(projectionResult))
        {
            throw std::invalid_argument("[ProjectionExecutor - Next]: Projection Expressions should not return bool");
        }
        if( (std::holds_alternative<int64_t>(projectionResult) && projection.projectionField.type != FieldType::INT64) || 
            (std::holds_alternative<std::string>(projectionResult) && projection.projectionField.type != FieldType::STRING32)
        )
        {
            throw std::invalid_argument("[ProjectionExecutor - Next]: Projection Expression Result should match Projection List types");
        }

        if(std::holds_alternative<int64_t>(projectionResult)) projectionValues.push_back(std::get<int64_t>(projectionResult));
        if(std::holds_alternative<std::string>(projectionResult)) projectionValues.push_back(std::get<std::string>(projectionResult));
    }

    out = Row{Tuple(std::move(projectionValues)), std::nullopt};
    return true;
}