#include "MaterialInstance.h"
#include "Material.h"

omp::MaterialInstance::MaterialInstance(const std::shared_ptr<omp::Material>& materialCreateFrom)
    : m_StaticMaterial(materialCreateFrom)
{

}

std::string omp::MaterialInstance::GetShaderName() const
{
    if (!m_StaticMaterial.expired())
    {
        return m_StaticMaterial.lock()->GetShaderName();
    }

    return "";
}
