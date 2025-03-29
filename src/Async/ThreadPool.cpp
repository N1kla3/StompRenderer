// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

#include "ThreadPool.h"

thread_local omp::InterruptFlag omp::Helper::g_ThisThreadInterruptFlag = {};

void omp::InterruptionPoint()
{
    if (omp::Helper::g_ThisThreadInterruptFlag.isSet())
    {
        throw omp::ThreadInterruptedException();
    }
}

template<typename Lockable>
void omp::InterruptibleWait(std::condition_variable_any& cond, Lockable& lockable)
{
    Helper::g_ThisThreadInterruptFlag.wait(cond, lockable);
}
