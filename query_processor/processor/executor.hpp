#pragma once
#include "schema.hpp" // Schema
#include "tuple.hpp" // Tuple == Row

class Exectuor
{
    public:
        virtual ~Executor() = default;
        virtual void Init() = 0;
        virtual bool Next(Tuple& out) = 0;
        virtual Close() = 0;
        virtual const Schema& OutputSchema() const = 0;

};