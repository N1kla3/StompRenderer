#include "ModelAsset.h"

omp::ModelAsset::ModelAsset()
    : Asset()
{

}

void omp::ModelAsset::serializeData(nlohmann::json &data)
{
    write_modelPath(data);
}

void omp::ModelAsset::deserializeData(const nlohmann::json &data)
{
    modelPath = read_modelPath(data);
}
