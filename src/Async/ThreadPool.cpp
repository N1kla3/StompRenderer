#include "ThreadPool.h"

void omp::InterruptionPoint()
{
    if (omp::g_ThisThreadInterruptFlag.isSet())
    {
        throw omp::ThreadInterruptedException();
    }
}
