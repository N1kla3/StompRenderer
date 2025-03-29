// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#pragma once
#include <assert.h>
#include "Logs.h"

#define OMP_ASSERT(condition, message) \
if (!condition)\
{ VERROR(LogCore, message); std::assert(condition); }

namespace omp
{
    struct CoreLib
    {
        static uint32_t generateId32();
        static uint64_t generateId64();
    };
}
