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
        inline static std::unordered_map<std::string, std::function<std::unique_ptr<SerializableObject>()>> s_CreationMap;
    public:

        template< typename T >
        void registerClass(const std::string& inClassName)
        {
            s_CreationMap.insert( {inClassName, []{ return std::make_unique<T>(); }} );
        }

        std::unique_ptr<SerializableObject> createSerializableObject(const std::string& inClassName)
        {
            if (s_CreationMap.find(inClassName) != s_CreationMap.end())
            {
                // Call lambda
                return s_CreationMap[inClassName]();
            }
            VWARN(LogAssetManager, "Cant find specified class {} while creating asset", inClassName);
            return nullptr;
        }

        ObjectFactory() = default;
        ~ObjectFactory() = default;
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

