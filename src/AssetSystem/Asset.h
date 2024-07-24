#pragma once

#include <memory>
#include <mutex>
#include <unordered_set>
#include "IO/SerializableObject.h"
#include "ObjectFactory.h"

namespace omp
{
    struct AssetHandle
    {
        using handle_type = omp::SerializableObject::SerializationId;
        handle_type id;

        AssetHandle(handle_type newId)
            : id(newId){}

        bool operator==(const AssetHandle& other) const
        {
            return id == other.id;
        }
        AssetHandle& operator=(handle_type other)
        {
            id = other;
            return *this;
        }

        static const AssetHandle INVALID_HANDLE;
    };
}

template<>
struct std::hash<omp::AssetHandle>
{
    std::size_t operator()(const omp::AssetHandle& handle) const
    {
        return std::hash<uint64_t>()(handle.id);
    }
};

namespace omp
{
    struct MetaData
    {
        AssetHandle::handle_type asset_id = 0;
        std::string asset_name = "";
        std::string path_on_disk = "";
        std::string class_id = "";
        std::unordered_set<AssetHandle::handle_type> dependencies;

        bool IsValid() const
        {
            return asset_id != 0 && !path_on_disk.empty() && !class_id.empty() && !asset_name.empty();
        }
        operator bool() const
        {
            return IsValid();
        }
    };

    class Asset : public std::enable_shared_from_this<Asset>
    {
    private:
        struct asset_equal
        {
            using is_transparent = void;

            bool operator()(const std::shared_ptr<Asset>& l, const std::shared_ptr<Asset>& r) const
            {
                return l->m_Metadata.asset_id == r->m_Metadata.asset_id;
            }

            bool operator()(const std::shared_ptr<Asset>& l, const AssetHandle& r) const
            {
                return l->m_Metadata.asset_id == r.id;
            }

            bool operator()(const AssetHandle& l, const std::shared_ptr<Asset>& r) const
            {
                return l.id == r->m_Metadata.asset_id;
            }
        };

        struct asset_hash
        {
            using hash_type = std::hash<omp::AssetHandle::handle_type>;
            using is_transparent = void;

            size_t operator()(const std::shared_ptr<omp::Asset>& r) const
            {
                return hash_type{}(r->m_Metadata.asset_id);
            }
            size_t operator()(const AssetHandle& r) const
            {
                return hash_type{}(r.id);
            }
        };

        std::unordered_set<std::shared_ptr<omp::Asset>, asset_hash, asset_equal> m_Parents;
        std::unordered_set<std::shared_ptr<omp::Asset>, asset_hash, asset_equal> m_Children;
        MetaData m_Metadata;
        JsonParser<> m_Parser;
        std::shared_ptr<SerializableObject> m_Object = nullptr;
        mutable std::mutex m_Access;

        bool m_IsLoaded = false;

    // Methods //
    // ======= //
    private:
        bool loadMetadata();
        bool tryLoadObject(ObjectFactory* factory);
        void createObject(ObjectFactory* factory);
        void addMetadataToObject(omp::Asset* asset, omp::SerializableObject::SerializationId id);
        bool unloadAsset();
        bool saveMetadata();
        bool saveAsset();
        void specifyFileData(JsonParser<>&& fileData);
        void specifyMetaData(omp::MetaData&& metadata);

        /*
         * @brief Additionally add self as parent
        */
        void addChild(const std::shared_ptr<omp::Asset>& asset);
        void addParent(const std::shared_ptr<omp::Asset>& asset);

    public:
        MetaData getMetaData() const;

        std::shared_ptr<omp::Asset> getChild(AssetHandle handle);
        std::shared_ptr<omp::Asset> getParent(AssetHandle handle);
        void resetHierarchy();

        std::shared_ptr<SerializableObject> getObject() const;
        template< typename T >
        std::shared_ptr<T> getObjectAs() const
        {
            return std::dynamic_pointer_cast<T>(m_Object);
        }
        std::shared_ptr<Asset> getptr()
        {
            return shared_from_this();
        }
        void addDependency(AssetHandle::handle_type handle);

        bool isLoaded() const { return m_IsLoaded; }

    // Constructors/operators //
    // ====================== //
    public:
        Asset() = default;
        Asset(JsonParser<>&& fileData);
        Asset(const Asset&) = delete;
        Asset(Asset&&) = delete;
        Asset& operator=(const Asset&) = delete;
        Asset& operator=(Asset&&) = delete;
        virtual ~Asset() = default;

        bool operator==(const Asset& inOther);
        bool operator!=(const Asset& inOther);

        friend class AssetManager;
    private:

        inline static const std::string ID_KEY = "ObjectID";
        inline static const std::string PATH_KEY = "DiscPath";
        inline static const std::string CLASS_NAME_KEY = "ClassName";
        inline static const std::string ASSET_NAME_KEY = "AssetName";
        inline static const std::string DEPENDENCIES_KEY = "Dependencies";
        inline static const std::string METADATA_KEY = "Metadata";
        inline static const std::string MAIN_DATA_KEY = "PlainData";
    };
}
