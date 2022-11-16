#include "ImguiUnit.h"

omp::ImguiUnit::ImguiUnit()
{
    s_ImguiUnits.push_back(this);
}

omp::ImguiUnit::~ImguiUnit()
{
    for (auto start = s_ImguiUnits.begin(); start != s_ImguiUnits.end(); start++)
    {
        if (*start == this)
        {
            s_ImguiUnits.erase(start);
            return;
        }
    }
}
