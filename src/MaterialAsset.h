#pragma once
#include "Asset.h"

namespace omp{
class MaterialAsset : public Asset
{
public:
    MaterialAsset(){}
protected:
    virtual void serializeData(nlohmann::json& data) override {};
    virtual void deserializeData(const nlohmann::json& data) override {};
};
};
