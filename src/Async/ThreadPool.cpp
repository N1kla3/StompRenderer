#include "ThreadPool.h"

thread_local omp::InterruptFlag omp::Helper::g_ThisThreadInterruptFlag = {};

void omp::InterruptionPoint()
{
    if (omp::Helper::g_ThisThreadInterruptFlag.isSet())
    {
        throw omp::ThreadInterruptedException();
    }
}

template< typename Lockable >
void omp::InterruptibleWait(std::condition_variable_any& cond, Lockable& lockable)
{
    Helper::g_ThisThreadInterruptFlag.wait(cond, lockable);
}
