#pragma once

#include <memory>
#include "ISaveable.h"
#include "UI/AssetRepresentation.h"
#include "nlohmann/json.hpp"

// BE CAREFUL, THIS APPLIES PUBLIC SPECIFIER
#define DECLARE_SERIALIZABLE_MEMBER(Type, Name) \
protected:                                                \
Type Name;                                    \
std::string Name##_Name = #Name;              \
void write_##Name(nlohmann::json& inJson)     \
{                                             \
    writeValueToJson<Type>(std::move(Name), inJson, Name##_Name); \
}                                              \
Type read_##Name(const nlohmann::json& inJson)        \
{                                                \
    return loadValueFromJson<Type>(inJson, Name##_Name);\
}                                               \
public:\
Type get##Name() const { return Name; }

namespace omp
{
    class ClassTypeBase;
    class Asset : public ISaveable
    {
    protected:
        std::string m_Name = "";
        std::string m_Path = "";
        // TODO find something more apropriate,
        std::string m_ClassName = "";
        std::shared_ptr<AssetRepresentation> m_AssetRepresentation;

    protected:
        virtual void initialize() = 0;
        virtual void serializeData(nlohmann::json& data) = 0;
        virtual void deserializeData(const nlohmann::json& data) = 0;

        template<typename T>
        void writeValueToJson(T&& inValue, nlohmann::json& inJson, const std::string& inName);
        template<typename T>
        T loadValueFromJson(const nlohmann::json& inJson, const std::string& inName);

        virtual void saveAssetToFile(const std::string& inPath, const std::string& inClassName) override;
    public:
        virtual ~Asset() = default;

        void setName(const std::string& inName) { m_Name = inName; }

        void setPath(const std::string& inPath) { m_Path = inPath; }

        bool saveToLastValidPath();

        std::string getName() const { return m_Name; }

        std::string getPath() const { return m_Path; }

        std::shared_ptr<AssetRepresentation> getVisualRepresentation() const { return m_AssetRepresentation; };

        // This is the only places to store data
        inline static const std::string TEXTURES_FOLDER = "../textures/";
        inline static const std::string ASSET_FOLDER = "../assets/";
        inline static const std::string MODELS_FOLDER = "../models/";

        inline static const std::string ASSET_FORMAT = ".json";

        inline static const std::string NAME_MEMBER = "Name";
        inline static const std::string CLASS_MEMBER = "Class";

        friend class AssetManager;
    };

    template<typename T>
    void Asset::writeValueToJson(T&& inValue, nlohmann::json& inJson, const std::string& inName)
    {
        inJson[inName] = std::forward<T>(inValue);
    }

    template<typename T>
    T Asset::loadValueFromJson(const nlohmann::json& inJson, const std::string& inName)
    {
        return inJson[inName].get<T>();
    }
}
