#include "SceneAsset.h"

omp::SceneAsset::SceneAsset()
        : Asset()
{

}

void omp::SceneAsset::initialize()
{

}

void omp::SceneAsset::serializeData(nlohmann::json& data)
{

}

void omp::SceneAsset::deserializeData(const nlohmann::json& data)
{

}

void omp::to_json(omp::json& j, const omp::ModelForSceneData& model)
{
    j = json{
        {"name", model.name},
        {"path", model.path},
        {"translation", model.translation},
        {"rotation", model.rotation},
        {"scale", model.scale}
    };
}

void omp::from_json(const omp::json& j, omp::ModelForSceneData& p)
{
    j.at("name").get_to(p.name);
    j.at("path").get_to(p.path);
    j.at("translation").get_to(p.translation);
    j.at("rotation").get_to(p.rotation);
    j.at("scale").get_to(p.scale);
}
