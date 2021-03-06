#include "MaterialManager.h"
#include "Logs.h"

omp::MaterialManager::MaterialManager(const std::shared_ptr<VulkanContext> &helper)
    : m_VkHelper(helper)
{

}

omp::MaterialManager::~MaterialManager()
{
    for (auto texture_pair : m_Textures)
    {
        texture_pair.second->DestroyVkObjects();
    }
}

std::shared_ptr<omp::Texture> omp::MaterialManager::LoadTextureInstantly(const std::string &path)
{
    auto texture_ptr = std::make_shared<Texture>(m_VkHelper.lock()->m_LogicalDevice, m_VkHelper.lock()->m_PhysDevice, m_VkHelper.lock());
    texture_ptr->FullLoad(path);
    m_Textures.insert({path, texture_ptr});
    return texture_ptr;
}

std::shared_ptr<omp::Texture> omp::MaterialManager::LoadTextureLazily(const std::string &path)
{
    auto texture_ptr = std::make_shared<Texture>(m_VkHelper.lock()->m_LogicalDevice, m_VkHelper.lock()->m_PhysDevice, m_VkHelper.lock());
    texture_ptr->LazyLoad(path);
    m_Textures.insert({path, texture_ptr});
    return texture_ptr;
}

std::shared_ptr<omp::Texture> omp::MaterialManager::GetTexture(const std::string &path) const
{
    if (m_Textures.find(path) != m_Textures.end())
    {
        return m_Textures.at(path);
    }
    WARN(Rendering, "Texture do not exists: " + path);
    return nullptr;
}

std::shared_ptr<omp::Material> omp::MaterialManager::CreateMaterial(const std::string& name)
{
    auto mat = std::make_shared<omp::Material>(name);
    m_Materials.insert({name, mat});
    return mat;
}
