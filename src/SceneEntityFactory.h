#pragma once
#include "SceneEntity.h"
#include <memory>
#include <type_traits>

namespace omp
{
    template< typename T >
    concept is_scene_entity = std::is_base_of_v<omp::SceneEntity, T>;

    class SceneEntityFactory final
    {
    private:
        inline static std::unordered_map<std::string, std::function<omp::SceneEntity*()>> m_CreationMap{};
    public:
        
        template< typename T >
        inline static void registerClass(const std::string& inClassName)
        {
            m_CreationMap.insert({ inClassName, []{ return new T{}; } });
        }

        

        template< is_scene_entity T = omp::SceneEntity>
        [[nodiscard]] inline static std::unique_ptr<T> createSceneEntity(const std::string& inClassName)
        {
            if (m_CreationMap.find(inClassName) != m_CreationMap.end())
            {
                auto* raw_ptr = m_CreationMap[inClassName]();
                T* casted = dynamic_cast<T*>(raw_ptr);
                if (casted)
                {
                    return std::unique_ptr<T>(casted);
                }
                else
                {
                    delete raw_ptr;
                    return nullptr;
                }
            }
            VWARN(LogAssetManager, "Cant find specified {} class while creating scene entity", inClassName);
            return nullptr;
        }

        SceneEntityFactory() = delete;
        ~SceneEntityFactory() = delete;
        SceneEntityFactory(const SceneEntityFactory&) = delete;
        SceneEntityFactory(SceneEntityFactory&&) = delete;
        SceneEntityFactory& operator=(const SceneEntityFactory&) = delete;
        SceneEntityFactory& operator=(SceneEntityFactory&&) = delete;
    };
}

