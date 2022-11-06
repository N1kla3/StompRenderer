#pragma once
#include "Asset.h"

namespace omp{
class ModelAsset : public Asset
{
public:
    ModelAsset(){}
    virtual void serializeData(nlohmann::json& data) override {};
    virtual void deserializeData(const nlohmann::json& data) override {};
};
};
