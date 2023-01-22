#include "ModelAsset.h"
#include "ModelStatics.h"
#include "MaterialManager.h"

omp::ModelAsset::ModelAsset()
        : Asset()
{

}

void omp::ModelAsset::initialize()
{
    m_Model = omp::ModelStatics::loadModel(m_Name, modelPath);
    m_Material = omp::MaterialManager::getMaterialManager().createOrGetMaterial(materialPath);
    m_Model->setMaterial(m_Material);
}

void omp::ModelAsset::serializeData(nlohmann::json& data)
{
    write_modelPath(data);
    write_materialPath(data);
}

void omp::ModelAsset::deserializeData(const nlohmann::json& data)
{
    modelPath = read_modelPath(data);
    materialPath = read_materialPath(data);
}
