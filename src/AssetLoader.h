#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include "MaterialAsset.h"
#include "ModelAsset.h"
#include "Asset.h"

#define IMPLEMENT_CLASS(ClassName)\
template<>\
struct Classes<ClassName>\
{\
    inline static const std::string TYPE = #ClassName;\
    using ClassType = ClassName;\
    inline static Asset* CreateClassAsset()\
    {\
        return Asset::createAsset<ClassName>();\
    }\
};\

#define CLASS_DETECTION_PAIR(ClassName) {AssetLoader::Classes<ClassName>::TYPE, &AssetLoader::Classes<ClassName>::CreateClassAsset}


namespace omp
{
    class AssetLoader
    {
        static Asset* loadAssetFromStorage(const std::string& path);
        static Asset* createClassFromString(const std::string& name);

        template<class T>
        struct Classes
        {
            inline static const std::string TYPE = "Undefined";
            using ClassType = T;

            inline static Asset* CreateClassAsset()
            {
                return Asset::createAsset<T>();
            }
        };

        IMPLEMENT_CLASS(MaterialAsset);

        IMPLEMENT_CLASS(ModelAsset);

        inline static const std::unordered_map<std::string, std::function<Asset*()>> CLASS_NAMES
                {
                        CLASS_DETECTION_PAIR(MaterialAsset),
                        CLASS_DETECTION_PAIR(ModelAsset)
                };
    public:
        AssetLoader() = delete;
        ~AssetLoader() = delete;
        AssetLoader& operator=(const AssetLoader&) = delete;
        AssetLoader& operator=(AssetLoader&&) = delete;

        friend class AssetManager;
    };
}


