#include <cstdint>
#include <mutex>
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include "Core/Profiling.h"
#include "AssetSystem/ObjectFactory.h"

const omp::AssetHandle omp::AssetHandle::INVALID_HANDLE = 0;

bool omp::Asset::loadMetadata()
{
    JsonParser<> metadata_parser = m_Parser.readObject(METADATA_KEY);
    m_Metadata.asset_id = metadata_parser.readValue<uint64_t>(ID_KEY).value_or(0);
    m_Metadata.path_on_disk = metadata_parser.readValue<std::string>(PATH_KEY).value();
    m_Metadata.class_id = metadata_parser.readValue<std::string>(CLASS_NAME_KEY).value();
    m_Metadata.asset_name = metadata_parser.readValue<std::string>(ASSET_NAME_KEY).value();
    m_Metadata.dependencies = metadata_parser.readValue<std::unordered_set<AssetHandle::handle_type>>(DEPENDENCIES_KEY).value();
    return m_Metadata.IsValid();
}

bool omp::Asset::tryLoadObject()
{
    OMP_STAT_SCOPE("LoadObject");

    std::lock_guard<std::mutex> lock(m_Access);
    if (m_Metadata && !m_IsLoaded)
    {
        m_Object = omp::ObjectFactory::createSerializableObject(m_Metadata.class_id);
        addMetadataToObject(this, m_Metadata.asset_id);
        JsonParser<> main_parser = m_Parser.readObject(MAIN_DATA_KEY);
        m_Object->deserialize(main_parser);
        m_IsLoaded = true;
        return true;
    }
    return false;
}

void omp::Asset::createObject()
{
    OMP_STAT_SCOPE("CreateObject");

    if (m_Metadata)
    {
        m_Object = omp::ObjectFactory::createSerializableObject(m_Metadata.class_id);
        if (m_Object)
        {
            addMetadataToObject(this, m_Metadata.asset_id);
            m_IsLoaded = true;
        }
    }
    else
    {
        WARN(LogAssetManager, "Invalid metadata to create asset from");
    }
}

bool omp::Asset::unloadAsset()
{
    std::lock_guard<std::mutex> lock(m_Access);
    if (m_Object)
    {
        m_Object.reset();
        m_IsLoaded = false;
        return true;
    }
    return false;
}

bool omp::Asset::saveMetadata()
{
    JsonParser<> metadata_parser;
    metadata_parser.writeValue(ID_KEY, m_Metadata.asset_id);
    metadata_parser.writeValue(PATH_KEY, m_Metadata.path_on_disk);
    metadata_parser.writeValue(CLASS_NAME_KEY, m_Metadata.class_id);
    metadata_parser.writeValue(ASSET_NAME_KEY, m_Metadata.asset_name);
    metadata_parser.writeValue(DEPENDENCIES_KEY, m_Metadata.dependencies);

    m_Parser.writeObject(METADATA_KEY, std::move(metadata_parser));
    return m_Metadata.IsValid();
}

bool omp::Asset::saveAsset()
{
    OMP_STAT_SCOPE("SaveAsset");

    if (m_Metadata)
    {
        std::lock_guard<std::mutex> lock(m_Access);
        if (m_Object)
        {

            JsonParser<> main_data_parser;
            m_Object->serialize(main_data_parser);
            m_Parser.writeObject(MAIN_DATA_KEY, std::move(main_data_parser));

            // Call after main serialization, because we need to know dependencies
            bool succ = saveMetadata();
            if (!succ)
            {
                WARN(LogAssetManager, "Metadata saving error");
            };

            return m_Parser.writeToFile(m_Metadata.path_on_disk);
        }
        else
        {
            WARN(LogAssetManager, "Cant deserialize object because it is not loaded");
            return false;
        }
    }
    return false;
}

omp::MetaData omp::Asset::getMetaData() const
{
    std::unique_lock lock(m_Access);
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
        asset->m_Parents.insert(getptr());
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
        asset->m_Children.insert(getptr());
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

void omp::Asset::resetHierarchy()
{
    m_Children.clear();
    m_Parents.clear();
}

void omp::Asset::addDependency(AssetHandle::handle_type handle)
{
    m_Metadata.dependencies.insert(handle);
    INFO(LogAssetManager, "DEPENDENCY ADDED {}", handle);
}

void omp::Asset::addMetadataToObject(omp::Asset* asset, omp::SerializableObject::SerializationId id)
{
    if (m_Object)
    {
        m_Object->m_SerializationId = id;
        m_Object->m_Asset = asset;
    }
}

