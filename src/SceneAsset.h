#pragma once
#include "Asset.h"

namespace omp {
class SceneAsset : public Asset
{
public:
    SceneAsset();
protected:
    virtual void initialize() override;
    virtual void serializeData(nlohmann::json& data) override;
    virtual void deserializeData(const nlohmann::json& data) override;

};
};
