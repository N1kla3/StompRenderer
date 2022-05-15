#pragma once

#include "Material.h"
#include "ImguiUnit.h"

namespace omp{
class MaterialPanel : public ImguiUnit
{
    std::weak_ptr<Material> m_Material;

public:

    virtual void renderUI() override;

    void setMaterial(const std::shared_ptr<Material>& inMaterial);
};
}