#include <filesystem>
#include <fstream>
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include "AssetSystem/ObjectFactory.h"

bool omp::Asset::loadMetadata()
{
    return false;
}

bool omp::Asset::loadAsset(ObjectFactory& factory)
{
    if (m_Metadata)
    {
        m_Object = factory.createSerializableObject(m_Metadata.class_id);
        m_Object->serialize(m_Parser);
        return true;
    }
    return false;
}

bool omp::Asset::unloadAsset()
{
    m_Object.reset();
    return true;
}

bool omp::Asset::saveAsset()
{
    if (m_Metadata)
    {
        //TODO: overwritten???
        //TODO: What if some keys are deleted? they still in json?
        if (m_Object)
        {
            m_Object->deserialize(m_Parser);
            return m_Parser.writeToFile(m_Metadata.path_on_disk);
        }
        else
        {
            ERROR(LogAssetManager, "Cant deserialize object while saving asset");
            return false;
        }
    }
    return false;
}

omp::MetaData omp::Asset::getMetaData() const
{
    return m_Metadata;
}

omp::SerializableObject* omp::Asset::getObject() const
{
    return m_Object.get();
}

omp::Asset::Asset(const std::string& inPathToAsset)
{

}

bool omp::Asset::operator==(const omp::Asset& inOther)
{
    return false;
}

bool omp::Asset::operator!=(const omp::Asset& inOther)
{
    return false;
}
