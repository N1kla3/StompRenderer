#pragma once

#include <memory>
#include <mutex>
#include "IO/SerializableObject.h"
#include "ObjectFactory.h"

namespace omp
{
    struct AssetHandle
    {
        using handle_type = uint64_t;
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
        uint64_t asset_id = 0;
        std::string asset_name = "none";
        std::string path_on_disk = "none";
        std::string class_id = "none";
        std::vector<AssetHandle::handle_type> dependencies;

        bool IsValid() const
        {
            return asset_id != 0 && !path_on_disk.empty() && path_on_disk.compare("none") != 0 && !class_id.empty() && class_id.compare("none") != 0;
        }
        operator bool() const
        {
            return IsValid();
        }
    };

    class Asset : public std::enable_shared_from_this<Asset>
    {
    private:
        std::vector<std::shared_ptr<omp::Asset>> m_Parents;
        std::vector<std::shared_ptr<omp::Asset>> m_Children;
        MetaData m_Metadata;
        JsonParser<> m_Parser;
        std::unique_ptr<SerializableObject> m_Object;
        mutable std::mutex m_Access;

    // Methods //
    // ======= //
    private:
        bool loadMetadata();
        bool loadAsset(ObjectFactory* factory);
        bool unloadAsset();
        bool saveMetadata();
        bool saveAsset();
        void specifyFileData(JsonParser<>&& fileData);

    public:
        MetaData getMetaData() const;

        /**
         * Asset have full response for lifecycle of that object
         *
         * @return Not manageable raw pointer to object that asset owns
         */
        SerializableObject const* getObject() const;
        std::shared_ptr<Asset> getptr()
        {
            return shared_from_this();
        }

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
    };
}
