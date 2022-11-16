#pragma once

#include "ImguiUnit.h"

namespace omp
{
    class MainLayer : public ImguiUnit
    {
    public:
        virtual void renderUi(float deltaTime) override;
    };
}

