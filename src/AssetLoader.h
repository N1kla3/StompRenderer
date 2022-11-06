#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "MaterialAsset.h"
#include "ModelAsset.h"
#include "Asset.h"

#define ImplementClass(ClassName)\
template<>\
struct Classes<ClassName>\
{\
    inline static const std::string Type = #ClassName;\
    using ClassType = ClassName;\
    inline static Asset* CreateClassAsset()\
    {\
        return Asset::CreateAsset<ClassName>();\
    }\
};\

#define ClassDetectionPair(ClassName) {AssetLoader::Classes<ClassName>::Type, &AssetLoader::Classes<ClassName>::CreateClassAsset}


namespace omp
{
    class AssetLoader
    {
        static Asset* LoadAssetFromStorage(const std::string& path);
        static Asset* CreateClassFromString(const std::string& name);

        template<class T>
        struct Classes
        {
            inline static const std::string Type = "Undefined";
            using ClassType = T;
            inline static Asset* CreateClassAsset()
            {
                return Asset::CreateAsset<T>();
            }
        };

        ImplementClass(MaterialAsset);
        ImplementClass(ModelAsset);

        inline static const std::unordered_map<std::string, std::function<Asset*()>> ClassNames
            {
                    ClassDetectionPair(MaterialAsset),
                    ClassDetectionPair(ModelAsset)
            };
    public:
        AssetLoader() = delete;
        ~AssetLoader() = delete;
        AssetLoader& operator=(const AssetLoader&) = delete;
        AssetLoader& operator=(AssetLoader&&) = delete;
        friend class Asset;
    };
}


