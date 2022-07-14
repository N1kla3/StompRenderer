#pragma once

#include <memory>
#include "ImguiUnit.h"


namespace omp{

class Material;

class MaterialPanel : public ImguiUnit
{
    std::weak_ptr<Material> m_Material;

public:

    virtual void renderUI(float DeltaTime) override;

    void setMaterial(const std::shared_ptr<Material>& inMaterial);
};
}