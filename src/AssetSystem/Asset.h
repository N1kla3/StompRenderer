#pragma once

#include <memory>
#include "IO/SerializableObject.h"
#include "ObjectFactory.h"

namespace omp
{
    struct MetaData
    {
        uint64_t asset_id = 0;
        std::string asset_name = "none";
        std::string path_on_disk = "none";
        std::string class_id = "none";

        bool IsValid() const
        {
            return asset_id != 0 && !path_on_disk.empty() && path_on_disk.compare("none") != 0 && !class_id.empty() && class_id.compare("none") != 0;
        }
        operator bool() const
        {
            return IsValid();
        }
    };

    class Asset
    {
    private:
        MetaData m_Metadata;
        JsonParser<> m_Parser;
        std::unique_ptr<SerializableObject> m_Object;

    // Methods //
    // ======= //
    private:
        bool loadMetadata();
        bool loadAsset(ObjectFactory& factory);
        bool unloadAsset();
        bool saveAsset();

    public:
        MetaData getMetaData() const;

        /**
         * Asset have full response for lifecycle of that object
         *
         * @return Not manageable raw pointer to object that asset owns
         */
        SerializableObject* getObject() const;

    // Constructors/operators //
    // ====================== //
    public:
        Asset() = default;
        Asset(const std::string& inPathToAsset);
        Asset(const Asset&) = delete;
        Asset(Asset&&) = delete;
        Asset& operator=(const Asset&) = delete;
        Asset& operator=(Asset&&) = delete;
        virtual ~Asset() = default;

        bool operator==(const Asset& inOther);
        bool operator!=(const Asset& inOther);

        friend class AssetManager;
    };
}