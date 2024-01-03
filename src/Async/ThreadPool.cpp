#include "ThreadPool.h"

void InterruptionPoint()
{
    if (omp::g_ThisThreadInterruptFlag.isSet())
    {
        throw omp::ThreadInterruptedException();
    }
}
