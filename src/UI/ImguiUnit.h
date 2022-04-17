#pragma once
#include <vector>

namespace omp{
/*
 * Base class for ui elements, which is automatically rendered, only need to override renderUI()
 */
class ImguiUnit
{
    inline static std::vector<ImguiUnit*> ImguiUnits{};

public:
    ImguiUnit();
    virtual ~ImguiUnit();

    virtual void renderUI() = 0;

    inline static std::vector<ImguiUnit*> GetAllUnits(){ return ImguiUnits; }
};
} // omp

