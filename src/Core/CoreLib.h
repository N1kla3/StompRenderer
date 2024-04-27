#pragma once
#include <assert.h>
#include "Logs.h"

#define OMP_ASSERT(condition, message) \
if (!condition)\
{ VERROR(LogCore, message); std::assert(condition); }\

namespace omp
{
    struct CoreLib
    {
        static uint32_t generateId32();
        static uint64_t generateId64();
    };
}
