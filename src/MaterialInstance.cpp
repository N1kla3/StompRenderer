#include "MaterialInstance.h"
#include "Rendering/Material.h"

omp::MaterialInstance::MaterialInstance(const std::shared_ptr<omp::Material>& materialCreateFrom)
        : m_StaticMaterial(materialCreateFrom)
{

}

std::string omp::MaterialInstance::getShaderName() const
{
    if (!m_StaticMaterial.expired())
    {
        return m_StaticMaterial.lock()->getShaderName();
    }

    return "";
}
