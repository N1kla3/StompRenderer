#include "AssetSystem/SceneAsset.h"
#include "AssetSystem/AssetManager.h"

omp::SceneAsset::SceneAsset()
        : Asset()
{

}

void omp::SceneAsset::initialize()
{
    m_Scene = std::make_shared<omp::Scene>();
    for (auto& model_data : Models)
    {
        auto&& model = omp::AssetManager::getAssetManager().tryGetAndLoadIfNot_casted<omp::Model>(model_data.path);
        if (model)
        {
            m_Scene->addEntityToScene(std::make_shared<omp::SceneEntity>(m_Name,
                                                                         std::make_shared<omp::ModelInstance>(model)));
        }
        else
        {
            VWARN(LogAssetManager, "Cant load asset of Model class {0}", model_data.path);
        }
    }

}

void omp::SceneAsset::serializeData(nlohmann::json& data)
{
    write_Models(data);
    write_Camera(data);
    write_Light(data);
}

void omp::SceneAsset::deserializeData(const nlohmann::json& data)
{
    read_Models(data);
    read_Camera(data);
    read_Light(data);
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

void omp::to_json(omp::json& j, const omp::GlobalLight& light)
{
    j = json{
            {"position", light.position_or_direction},
            {"ambient", light.ambient},
            {"diffuse", light.diffuse},
            {"specular", light.specular},
    };
}

void omp::from_json(const omp::json& j, omp::GlobalLight& light)
{
    j.at("position").get_to(light.position_or_direction);
    j.at("ambient").get_to(light.ambient);
    j.at("diffuse").get_to(light.diffuse);
    j.at("specular").get_to(light.specular);
}

void omp::to_json(omp::json& j, const omp::CameraForSceneData& camera)
{
    j = json{
            {"position", camera.position},
            {"up", camera.up},
            {"pitch", camera.pitch},
            {"yaw", camera.yaw},
    };
}

void omp::from_json(const omp::json& j, omp::CameraForSceneData& camera)
{
    j.at("position").get_to(camera.position);
    j.at("up").get_to(camera.up);
    j.at("yaw").get_to(camera.yaw);
    j.at("pitch").get_to(camera.pitch);
}
