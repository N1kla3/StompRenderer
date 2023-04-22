#include "MaterialInstance.h"
#include "Rendering/Material.h"

omp::MaterialInstance::MaterialInstance(const std::shared_ptr<omp::Material>& materialCreateFrom)
        : m_StaticMaterial(materialCreateFrom)
{

}
