#include <cstdint>
#include <mutex>
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include "AssetSystem/ObjectFactory.h"

bool omp::Asset::loadMetadata()
{
    JsonParser<> metadata_parser = m_Parser.readObject("Metadata");
    m_Metadata.asset_id = metadata_parser.readValue<uint64_t>(ID_KEY).value_or(0);
    m_Metadata.path_on_disk = metadata_parser.readValue<std::string>(PATH_KEY).value();
    m_Metadata.class_id = metadata_parser.readValue<std::string>(CLASS_NAME_KEY).value();
    m_Metadata.asset_name = metadata_parser.readValue<std::string>(ASSET_NAME_KEY).value();
    m_Metadata.dependencies = metadata_parser.readValue<std::unordered_set<AssetHandle::handle_type>>(DEPENDENCIES_KEY).value();
    return m_Metadata.IsValid();
}

bool omp::Asset::loadAsset(ObjectFactory* factory)
{
    std::lock_guard<std::mutex> lock(m_Access);
    if (m_Metadata)
    {
        m_Object = factory->createSerializableObject(m_Metadata.class_id);
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

bool omp::Asset::saveMetadata()
{
    JsonParser<> metadata_parser;
    metadata_parser.writeValue(ID_KEY, m_Metadata.asset_id);
    metadata_parser.writeValue(PATH_KEY, m_Metadata.path_on_disk);
    metadata_parser.writeValue(CLASS_NAME_KEY, m_Metadata.class_id);
    metadata_parser.writeValue(ASSET_NAME_KEY, m_Metadata.asset_name);
    metadata_parser.writeValue(DEPENDENCIES_KEY, m_Metadata.dependencies);

    m_Parser.writeObject("Metadata", std::move(metadata_parser));
    return m_Metadata.IsValid();
}

bool omp::Asset::saveAsset()
{
    if (m_Metadata)
    {
        // TODO:: overwritten???
        // TODO : What if some keys are deleted? they still in json?
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

std::shared_ptr<omp::SerializableObject> omp::Asset::getObject() const
{
    return m_Object;
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

void omp::Asset::specifyMetaData(omp::MetaData&& metadata)
{
    m_Metadata = metadata;
}

void omp::Asset::addChild(const std::shared_ptr<omp::Asset>& asset)
{
    if (asset.get())
    {
        asset->addParent(getptr());
        m_Children.insert(asset);
    }
    else
    {
        ERROR(LogAssetManager, "Cant add child to asset: {}, with id: ", m_Metadata.asset_name, m_Metadata.class_id);
    }
}

void omp::Asset::addParent(const std::shared_ptr<omp::Asset>& asset)
{
    if (asset.get())
    {
        m_Parents.insert(asset);
    }
    else
    {
        ERROR(LogAssetManager, "Cant add parent to asset: {}, with id: ", m_Metadata.asset_name, m_Metadata.class_id);
    }
}

std::shared_ptr<omp::Asset> omp::Asset::getChild(AssetHandle handle)
{
    auto iter = m_Children.find<omp::AssetHandle>(handle);
    if (iter != m_Children.end())
    {
        return *iter;
    }
    return nullptr;
}

std::shared_ptr<omp::Asset> omp::Asset::getParent(AssetHandle handle)
{
    auto iter = m_Parents.find<omp::AssetHandle>(handle);
    if (iter != m_Parents.end())
    {
        return *iter;
    }
    return nullptr;
}

void omp::Asset::addDependency(AssetHandle::handle_type handle)
{
    m_Metadata.dependencies.insert(handle);
}

