#pragma once
#include "Asset.h"
#include "Model.h"

namespace omp
{
    class ModelAsset : public Asset
    {
        std::shared_ptr<omp::Model> m_Model;
        std::shared_ptr<omp::Material> m_Material;
    public:
        ModelAsset();
    protected:
        virtual void initialize() override;
        virtual void serializeData(nlohmann::json& data) override;
        virtual void deserializeData(const nlohmann::json& data) override;

        DECLARE_SERIALIZABLE_MEMBER(std::string, ModelPath);
        DECLARE_SERIALIZABLE_MEMBER(std::string, MaterialPath);

    };
}
