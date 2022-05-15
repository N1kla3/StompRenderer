#pragma once

#include <memory>
#include "ISaveable.h"
#include "UI/AssetRepresentation.h"

namespace omp{
class Asset : public ISaveable
{
private:
    std::string m_Name;
    std::shared_ptr<AssetRepresentation> m_AssetRepresentation;
public:
    virtual std::shared_ptr<AssetRepresentation> getVisualRepresentation() const { return m_AssetRepresentation; };
    virtual void initialize() = 0;
    virtual void saveAssetToFile(const std::string& path) override;
    virtual void loadAssetFromFile(const std::string &path) override;

    void SetName(const std::string& inName) { m_Name = inName; }

    // This is the only places to store data
    inline static const std::string TEXTURES_FOLDER = "../textures/";
    inline static const std::string ASSET_FOLDER = "../assets/";
    inline static const std::string MODELS_FOLDER = "../models/";

    inline static const std::string ASSET_FORMAT = ".ass";
};
}