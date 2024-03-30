#pragma once

#include "ImguiUnit.h"
#include "IDrawable.h"

namespace omp
{
class EntityPanel : public ImguiUnit
    {
        IDrawable* m_Entity = nullptr;

    public:
        virtual void renderUi(float deltaTime) override;
        void setEntity(IDrawable* inEntity);
    };
}


