#pragma once
#include "SceneEntity.h"
#include <memory>


namespace omp
{
    class SceneEntityFactory final
    {
    private:
        inline static std::unordered_map<std::string, std::function<std::unique_ptr<omp::SceneEntity>()>> m_CreationMap{};
    public:
        
        template< typename T >
        inline static void registerClass(const std::string& inClassName)
        {
            m_CreationMap.insert({ inClassName, []{ return std::make_unique<T>(); } });
        }

        [[nodiscard]] inline static std::unique_ptr<omp::SceneEntity> createSceneEntity(const std::string& inClassName)
        {
            if (m_CreationMap.find(inClassName) != m_CreationMap.end())
            {
                return m_CreationMap[inClassName]();
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

