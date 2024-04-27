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
            m_RenderInfo->textures.erase(m_RenderInfo->textures.cbegin() + index);
            break;
        }
    }
    clearDescriptorSets();
}

void omp::Material::addTexture(const std::shared_ptr<omp::TextureSrc>& texture)
{ 
    // TODO: refactor texture to suit texture src
    std::shared_ptr<omp::Texture> texture_inst = std::make_shared<omp::Texture>();
    addTexture(ETextureType::Texture, texture_inst);
}

void omp::Material::addDiffusiveTexture(const std::shared_ptr<omp::TextureSrc>& texture) 
{
    std::shared_ptr<omp::Texture> texture_inst = std::make_shared<omp::Texture>();
    addTexture(ETextureType::DiffusiveMap, texture_inst);
}

void omp::Material::addSpecularTexture(const std::shared_ptr<omp::TextureSrc>& texture) 
{
    std::shared_ptr<omp::Texture> texture_inst = std::make_shared<omp::Texture>();
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
    //parser.writeValue("texture", serializeDependency(m_RenderInfo->textures[static_cast<size_t>(ETextureType::Texture)].get()));
    //parser.writeValue("diffuse_map", serializeDependency(m_DiffusiveMap.get()));
    //parser.writeValue("specular_map", serializeDependency(m_SpecularMap.get()));
    // TODO: maybe try another approach without string
    parser.writeValue("shader_name", m_RenderInfo->shader_name);
    parser.writeValue("enable_blending", m_EnableBlending);
}

void omp::Material::deserialize(JsonParser<> &parser)
{
    //m_Texture = std::dynamic_pointer_cast<omp::TextureSrc>(getDependency(parser.readValue<omp::SerializableObject::SerializationId>("texture").value()));
    //m_DiffusiveMap = std::dynamic_pointer_cast<omp::TextureSrc>(getDependency(parser.readValue<omp::SerializableObject::SerializationId>("diffuse_map").value()));
    //m_SpecularMap = std::dynamic_pointer_cast<omp::TextureSrc>(getDependency(parser.readValue<omp::SerializableObject::SerializationId>("specular_map").value()));

    m_RenderInfo->shader_name = parser.readValue<std::string>("shader_name").value();
    m_EnableBlending = parser.readValue<bool>("enable_blending").value();
}

void omp::Material::enableBlending(bool enable)
{
    m_EnableBlending = enable;
}
