#pragma once

#include "Asset.h"
#include "glm/vec3.hpp"
#include "Light.h"
#include "Scene.h"

namespace omp
{
    struct ModelForSceneData
    {
        std::string name;
        std::string path;

        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;
    };
    struct CameraForSceneData
    {
        glm::vec3 position;
        glm::vec3 up;
        float yaw;
        float pitch;
    };

    using json = nlohmann::json;
    void to_json(json& j, const omp::ModelForSceneData& model);
    void from_json(const json& j, omp::ModelForSceneData& p);
    void to_json(json& j, const omp::Light& light);
    void from_json(const json& j, omp::Light& light);
    void to_json(json& j, const omp::CameraForSceneData& camera);
    void from_json(const json& j, omp::CameraForSceneData& camera);

    class SceneAsset : public Asset
    {
    private:
        std::shared_ptr<omp::Scene> m_Scene;
    public:
        SceneAsset();
    protected:
        virtual void initialize() override;
        virtual void serializeData(nlohmann::json& data) override;
        virtual void deserializeData(const nlohmann::json& data) override;

        DECLARE_SERIALIZABLE_MEMBER(std::vector<omp::ModelForSceneData>, Models);
        DECLARE_SERIALIZABLE_MEMBER(omp::CameraForSceneData, Camera);
        DECLARE_SERIALIZABLE_MEMBER(omp::Light, Light);
    };
};
