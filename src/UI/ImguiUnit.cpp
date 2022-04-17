#include "ImguiUnit.h"

omp::ImguiUnit::ImguiUnit()
{
    ImguiUnits.push_back(this);
}

omp::ImguiUnit::~ImguiUnit()
{
    for (auto start = ImguiUnits.begin(); start != ImguiUnits.end(); start++)
    {
        if (*start == this)
        {
            ImguiUnits.erase(start);
            return;
        }
    }
}
