#include <mutex>
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include "AssetSystem/ObjectFactory.h"

bool omp::Asset::loadMetadata()
{
    m_Metadata.asset_id = m_Parser.readValue<uint64_t>("ObjectID").value_or(0);
    m_Metadata.path_on_disk = m_Parser.readValue<std::string>("DiscPath").value();
    m_Metadata.class_id = m_Parser.readValue<std::string>("ClassName").value();
    m_Metadata.asset_name = m_Parser.readValue<std::string>("AssetName").value();
    return true;
}

bool omp::Asset::loadAsset(ObjectFactory& factory)
{
    std::lock_guard<std::mutex> lock(m_Access);
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
    std::lock_guard<std::mutex> lock(m_Access);
    m_Object.reset();
    return true;
}

bool omp::Asset::saveAsset()
{
    if (m_Metadata)
    {
        //TODO: overwritten???
        //TODO: What if some keys are deleted? they still in json?
        std::lock_guard<std::mutex> lock(m_Access);
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

omp::Asset::Asset(omp::JsonParser<>&& fileData)
    : m_Parser(std::move(fileData))
{
}

bool omp::Asset::operator==(const omp::Asset& inOther)
{
    return m_Metadata.asset_id == inOther.m_Metadata.asset_id;
}

bool omp::Asset::operator!=(const omp::Asset& inOther)
{
    return !(*this == inOther);
}

void omp::Asset::specifyFileData(omp::JsonParser<>&& fileData)
{
    m_Parser = std::move(fileData);
}
