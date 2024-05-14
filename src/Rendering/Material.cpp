#include "Material.h"
#include "Logs.h"
#include "MaterialManager.h"
#include "UI/MaterialRepresentation.h"
#include <memory>

void omp::Material::addTextureInternal(TextureData&& data)
{
    clearDescriptorSets();
    m_RenderInfo->textures[data.binding_index] = std::move(data);
}

void omp::Material::addTexture(ETextureType type, const std::shared_ptr<Texture>& texture)
{
    const static std::array<std::string, static_cast<size_t>(ETextureType::Max)> names
            {
             "texture",
             "diffusive_map",
             "Specular_map"
            };

    clearDescriptorSets();

    addTextureInternal({static_cast<uint32_t>(type), texture, names[static_cast<uint32_t>(type)]});
}

void omp::Material::removeTexture(const TextureData& data)
{
    for (size_t index = 0; index < m_RenderInfo->textures.size(); index++)
    {
        if (m_RenderInfo->textures[index].binding_index == data.binding_index)
        {
            m_RenderInfo->textures.erase(m_RenderInfo->textures.cbegin() + static_cast<int>(index));
            break;
        }
    }
    clearDescriptorSets();
}

void omp::Material::addTexture(const std::shared_ptr<omp::TextureSrc>& texture)
{ 
    std::shared_ptr<omp::Texture> texture_inst = std::make_shared<omp::Texture>(texture);
    addTexture(ETextureType::Texture, texture_inst);
}

void omp::Material::addDiffusiveTexture(const std::shared_ptr<omp::TextureSrc>& texture) 
{
    std::shared_ptr<omp::Texture> texture_inst = std::make_shared<omp::Texture>(texture);
    addTexture(ETextureType::DiffusiveMap, texture_inst);
}

void omp::Material::addSpecularTexture(const std::shared_ptr<omp::TextureSrc>& texture) 
{
    std::shared_ptr<omp::Texture> texture_inst = std::make_shared<omp::Texture>(texture);
    addTexture(ETextureType::SpecularMap, texture_inst);
}

void omp::Material::setDescriptorSet(const std::vector<VkDescriptorSet>& ds)
{
    m_DescriptorSets = ds;
}

std::vector<VkDescriptorSet>& omp::Material::getDescriptorSet()
{
    return m_DescriptorSets;
}

std::vector<omp::TextureData> omp::Material::getTextureData() const
{
    return m_RenderInfo->textures;
}

omp::Material::Material()
{
    m_RenderInfo = std::make_unique<omp::MaterialRenderInfo>();
}

omp::Material::Material(const std::string& /*name*/)
    : Material()
{

}

void omp::Material::serialize(JsonParser<> &parser)
{
    auto texture = m_RenderInfo->textures[static_cast<size_t>(ETextureType::Texture)].texture;
    if (texture)
    {
        parser.writeValue("texture", serializeDependency(texture->getTextureSrc()));
    }
    auto diffus = m_RenderInfo->textures[static_cast<size_t>(ETextureType::DiffusiveMap)].texture;
    if (diffus)
    {
        parser.writeValue("diffuse_map", serializeDependency(diffus->getTextureSrc()));
    }
    auto spec = m_RenderInfo->textures[static_cast<size_t>(ETextureType::SpecularMap)].texture;
    if (spec)
    {
        parser.writeValue("specular_map", serializeDependency(spec->getTextureSrc()));
    }

    // TODO: maybe try another approach without string
    parser.writeValue("shader_name", m_RenderInfo->shader_name);
    parser.writeValue("enable_blending", m_EnableBlending);
}

void omp::Material::deserialize(JsonParser<> &parser)
{
    using id_type = omp::SerializableObject::SerializationId;
    auto texture = std::dynamic_pointer_cast<omp::TextureSrc>(getDependency(parser.readValue<id_type>("texture").value()));
    if (texture)
    {
        addTexture(texture);
    }
    auto diffusive_map = std::dynamic_pointer_cast<omp::TextureSrc>(getDependency(parser.readValue<id_type>("diffuse_map").value()));
    if (diffusive_map)
    {
        addDiffusiveTexture(diffusive_map);
    }
    auto specular_map = std::dynamic_pointer_cast<omp::TextureSrc>(getDependency(parser.readValue<id_type>("specular_map").value()));
    if (specular_map)
    {
        addSpecularTexture(specular_map);
    }

    m_RenderInfo->shader_name = parser.readValue<std::string>("shader_name").value();
    m_EnableBlending = parser.readValue<bool>("enable_blending").value();
}

void omp::Material::enableBlending(bool enable)
{
    m_EnableBlending = enable;
}
