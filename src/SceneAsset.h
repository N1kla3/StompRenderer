#pragma once

#include "Asset.h"
#include "glm/vec3.hpp"

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
    struct GlobalLightForScene
    {

    };

    using json = nlohmann::json;
    void to_json(json& j, const omp::ModelForSceneData& model);
    void from_json(const json& j, omp::ModelForSceneData& p);

    class SceneAsset : public Asset
    {
    public:
        SceneAsset();
    protected:
        virtual void initialize() override;
        virtual void serializeData(nlohmann::json& data) override;
        virtual void deserializeData(const nlohmann::json& data) override;

        DECLARE_SERIALIZABLE_MEMBER(omp::ModelForSceneData, Models);
    };
};
