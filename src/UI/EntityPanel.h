#pragma once

#include <memory>
#include "ImguiUnit.h"
#include "Rendering/Model.h"
#include "MaterialPanel.h"
#include "Rendering/ModelInstance.h"
#include "IDrawable.h"

namespace omp
{
class EntityPanel : public ImguiUnit
    {
        std::weak_ptr<IDrawable> m_Entity;

    public:
        virtual void renderUi(float deltaTime) override;
        void setEntity(const std::shared_ptr<IDrawable>& inEntity);
    };
}


