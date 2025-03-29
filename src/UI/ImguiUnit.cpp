// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

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
