#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include "AssetSystem/MaterialAsset.h"
#include "AssetSystem/ModelAsset.h"
#include "AssetSystem/Asset.h"

#define ADD_CLASS(ClassName) { typeid(std::decay_t<ClassName>).name(), new omp::ClassType<ClassName>() }

namespace omp
{
    struct ClassTypeBase
    {
        virtual std::shared_ptr<Asset> CreateAsset() const = 0;
        virtual ~ClassTypeBase() = default;
    };

    template<class T>
    requires std::is_base_of_v<Asset, T> && std::is_default_constructible_v<T>
    struct ClassType : public ClassTypeBase
    {
        using Type = T;

        virtual std::shared_ptr<Asset> CreateAsset() const override
        {
            return std::make_shared<Type>();
        }
    };

    class AssetLoader
    {
        static std::shared_ptr<omp::Asset> loadAssetFromStorage(const std::string& path);
        static std::shared_ptr<omp::Asset> createClassFromString(const std::string& name);

    public:
        inline static std::unordered_map<std::string, ClassTypeBase*> s_AssetClasses
                {
                        ADD_CLASS(MaterialAsset),
                        ADD_CLASS(ModelAsset)
                };

        AssetLoader() = delete;
        ~AssetLoader() = delete;
        AssetLoader& operator=(const AssetLoader&) = delete;
        AssetLoader& operator=(AssetLoader&&) = delete;

        template<class T>
        static std::string getClassString()
        {
            return typeid(std::decay_t<T>).name();
        }

        friend class AssetManager;
    };
}


