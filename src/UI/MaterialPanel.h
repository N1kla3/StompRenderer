#pragma once

#include <memory>
#include "ImguiUnit.h"


namespace omp{

class MaterialInstance;

class MaterialPanel : public ImguiUnit
{
    std::weak_ptr<MaterialInstance> m_MaterialInstance;

public:

    virtual void renderUI(float DeltaTime) override;

    void setMaterial(const std::shared_ptr<MaterialInstance>& inMaterial);
};
}