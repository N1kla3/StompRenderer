#include "ThreadPool.h"

void interruption_point()
{
    if (this_thread_interrupt_flag.is_set())
    {
        throw thread_interrupted();
    }
}
