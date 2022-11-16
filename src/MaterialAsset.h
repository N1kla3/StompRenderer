#pragma once

#include "Asset.h"

namespace omp
{
    class MaterialAsset : public Asset
    {
    public:
        MaterialAsset();
    protected:
        virtual void initialize() override;
        virtual void serializeData(nlohmann::json& data) override;
        virtual void deserializeData(const nlohmann::json& data) override;

        DECLARE_SERIALIZABLE_MEMBER(std::vector<std::string>, TexturePaths);
        DECLARE_SERIALIZABLE_MEMBER(std::string, ShaderName);
    };
}
