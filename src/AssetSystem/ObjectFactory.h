#pragma once

#include "IO/SerializableObject.h"
#include <string>
#include <unordered_map>
#include <functional>

// DEPRECATED
//#define ADD_CLASS(ClassName) { typeid(std::decay_t<ClassName>).name(), new omp::ClassType<ClassName>() }

namespace omp
{
    class ObjectFactory final
    {
    private:
        inline static std::unordered_map<std::string, std::function<std::unique_ptr<SerializableObject>()>> m_CreationMap{};
    public:

        template< typename T >
        inline static void registerClass(const std::string& inClassName)
        {
            m_CreationMap.insert( {inClassName, []{ return std::make_unique<T>(); }} );
        }

        [[nodiscard]] inline static std::unique_ptr<SerializableObject> createSerializableObject(const std::string& inClassName)
        {
            if (m_CreationMap.find(inClassName) != m_CreationMap.end())
            {
                // Call lambda
                return m_CreationMap[inClassName]();
            }
            VWARN(LogAssetManager, "Cant find specified class {} while creating asset", inClassName);
            return nullptr;
        }

        ObjectFactory() = delete;
        ~ObjectFactory() = delete;
        ObjectFactory(const ObjectFactory&) = delete;
        ObjectFactory(ObjectFactory&&) = delete;
        ObjectFactory& operator=(const ObjectFactory&) = delete;
        ObjectFactory& operator=(ObjectFactory&&) = delete;

        friend class AssetManager;
    };
}

/*
template<class T>
static std::string getClassString()
{
    return typeid(std::decay_t<T>).name();
}
*/

