#pragma once

#include <vector>

namespace omp
{
/*
 * Base class for ui elements, which is automatically rendered, only need to override renderUI()
 */
    class ImguiUnit
    {
        inline static std::vector<ImguiUnit*> s_ImguiUnits{};

    public:
        ImguiUnit();
        virtual ~ImguiUnit();

        // TODO think about delta time
        virtual void renderUi(float DeltaTime) = 0;

        inline static std::vector<ImguiUnit*> getAllUnits() { return s_ImguiUnits; }
    };
} // omp

