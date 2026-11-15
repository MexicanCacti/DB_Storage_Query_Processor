#pragma once
#include "types.hpp"

class Executor
{
    private:

    public:
        virtual ~Executor() = default;
        virtual void Init() = 0;
        virtual bool Next(Row& out) = 0;
        virtual Close() = 0;
        virtual const Schema& OutputSchema() const = 0;

};