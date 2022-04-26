#include "MaterialManager.h"

omp::MaterialManager::MaterialManager(const std::shared_ptr<VulkanHelper> &helper)
    : m_VkHelper(helper)
{

}

omp::MaterialManager::~MaterialManager()
{
    for (auto texture_pair : m_Textures)
    {
        texture_pair.second->Destroy();
    }
}

void omp::MaterialManager::LoadTextureInstantly(const std::string &path)
{
    auto texture_ptr = std::make_shared<Texture>(m_VkHelper.lock()->m_LogicalDevice, m_VkHelper.lock()->m_PhysDevice, m_VkHelper.lock());
    texture_ptr->FullLoad(path);
    m_Textures.insert({path, texture_ptr});
}

void omp::MaterialManager::LoadTextureLazily(const std::string &path)
{
    auto texture_ptr = std::make_shared<Texture>(m_VkHelper.lock()->m_LogicalDevice, m_VkHelper.lock()->m_PhysDevice, m_VkHelper.lock());
    texture_ptr->LazyLoad(path);
    m_Textures.insert({path, texture_ptr});
}

std::shared_ptr<omp::Texture> omp::MaterialManager::GetTexture(const std::string &path) const
{
    if (m_Textures.find(path) != m_Textures.end())
    {
        return m_Textures.at(path);
    }
    return nullptr;
}
