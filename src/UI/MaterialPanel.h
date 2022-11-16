#pragma once

#include <memory>
#include "ImguiUnit.h"


namespace omp
{

    class MaterialInstance;

    class MaterialPanel : public ImguiUnit
    {
        std::weak_ptr<MaterialInstance> m_MaterialInstance;

    public:

        virtual void renderUi(float deltaTime) override;

        void setMaterial(const std::shared_ptr<MaterialInstance>& inMaterial);
    };
}