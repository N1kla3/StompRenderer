#pragma once
#include <memory>
#include "ISaveable.h"
#include "UI/AssetRepresentation.h"
#include "nlohmann/json.hpp"

namespace omp{
class Asset : public ISaveable
{
protected:
    std::string m_Name;
    std::string m_Path;
    std::shared_ptr<AssetRepresentation> m_AssetRepresentation;
protected:
    virtual void serializeData(nlohmann::json& data) = 0;
    virtual void deserializeData(const nlohmann::json& data) = 0;
private:
    virtual void saveAssetToFile(const std::string& path) override;
    virtual void loadAssetFromFile(const std::string &path) override;
public:
    virtual std::shared_ptr<AssetRepresentation> getVisualRepresentation() const { return m_AssetRepresentation; };

    template<class T>
    requires std::is_base_of_v<Asset, T> && std::is_default_constructible_v<T>
    static T* CreateAsset(){ return new T(); }

    void SetName(const std::string& inName) { m_Name = inName; }
    void saveToLastValidPath() { saveAssetToFile(m_Path); }

    // This is the only places to store data
    inline static const std::string TEXTURES_FOLDER = "../textures/";
    inline static const std::string ASSET_FOLDER = "../assets/";
    inline static const std::string MODELS_FOLDER = "../models/";

    inline static const std::string ASSET_FORMAT = ".ass";
};
}