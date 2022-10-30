#pragma once
#include <string>
#include <unordered_map>
#include "Asset.h"

#define ImplementClass(ClassName)\
template<>\
struct omp::AssetLoader::Classes<int>\
{\
    inline static const std::string Type = #ClassName;\
    inline static bool IsSame(const std::string& CheckName)\
    {\
        return Type == CheckName;\
    }\
};\

#define ClassDetectionPair(ClassName) {AssetLoader::Classes<ClassName>::Type, &AssetLoader::Classes<ClassName>::IsSame}


namespace omp
{
    class AssetLoader
    {

        void LoadAssetFromStorage(const std::string& path);

        template<class T>
        struct Classes
        {
            inline static const std::string Type = "Undefined";
            inline static bool IsSame(const std::string& CheckName)
            {
                static_assert(std::is_base_of_v<Asset, T>);

                return Type == CheckName;
            }
        };

        ImplementClass(int)

        inline static const std::unordered_map<std::string, bool(*)(const std::string&)> ClassNames
            {
                    ClassDetectionPair(int)
            };
    };
}


