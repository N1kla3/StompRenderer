#include "ModelAsset.h"
#include "ModelStatics.h"
#include "MaterialManager.h"

omp::ModelAsset::ModelAsset()
        : Asset()
{

}

void omp::ModelAsset::initialize()
{
    m_Model = omp::ModelStatics::LoadModel(m_Name, ModelPath);
    m_Material = omp::MaterialManager::getMaterialManager().createOrGetMaterial(MaterialPath);
    m_Model->setMaterial(m_Material);
}

void omp::ModelAsset::serializeData(nlohmann::json& data)
{
    write_ModelPath(data);
    write_MaterialPath(data);
}

void omp::ModelAsset::deserializeData(const nlohmann::json& data)
{
    ModelPath = read_ModelPath(data);
    MaterialPath = read_MaterialPath(data);
}
