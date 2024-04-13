#include "IO/SerializableObject.h"
#include "AssetSystem/Asset.h"

void omp::SerializableObject::setAsset(Asset* asset)
{
    m_Asset = asset;
}

void omp::SerializableObject::setId(uint64_t id)
{
    m_SerializationId = id;
}

omp::SerializableObject::SerializationId omp::SerializableObject::serializeDependency(SerializableObject* object)
{
    m_Asset->addDependency(object->m_SerializationId);
    return object->m_SerializationId;
}

std::shared_ptr<omp::SerializableObject> omp::SerializableObject::getDependency(omp::SerializableObject::SerializationId id)
{
    std::shared_ptr<omp::Asset> asset = m_Asset->getChild(id);
    if (asset)
    {
        return asset->getObject();
    }
    return nullptr;
}

