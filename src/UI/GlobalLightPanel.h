#pragma once

#include <memory>
#include "ImguiUnit.h"

namespace omp
{

    struct GlobalLight;

    class GlobalLightPanel : public ImguiUnit
    {
    private:
        std::weak_ptr<omp::GlobalLight> m_LightRef;

    public:
        explicit GlobalLightPanel(const std::shared_ptr<omp::GlobalLight>& inLight);

        virtual void renderUi(float deltaTime);
    };
}
