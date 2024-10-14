#pragma once

#include "IO/JsonParser.h"

namespace omp 
{
    class Asset;

    class SerializableObject 
    {
    public:
        using SerializationId = uint64_t;
    private:
        SerializationId m_SerializationId;
        Asset* m_Asset = nullptr;
        void setAsset(Asset* asset);
        void setId(uint64_t id);

    public:
        SerializableObject() = default;
        virtual void serialize(JsonParser<> &parser) = 0;
        virtual void deserialize(JsonParser<> &parser) = 0;
        virtual ~SerializableObject() = default;

        SerializationId serializeDependency(SerializableObject* object);
        std::shared_ptr<SerializableObject> getDependency(SerializationId);

        friend class Asset;
    };
} // namespace omp
