#include "MaterialManager.h"
#include "Logs.h"

/*
std::shared_ptr<omp::Texture> omp::MaterialManager::loadTextureInstantly(const std::string& path)
{
     if (m_Textures.find(path) != m_Textures.end())
    {
        return m_Textures.at(path);
    }

    auto texture_ptr = std::make_shared<Texture>(path);
    if (!m_VulkanContext.expired())
    {
        texture_ptr->specifyVulkanContext(m_VulkanContext.lock());
    }
    texture_ptr->fullLoad();
    m_Textures.insert({path, texture_ptr});
    return texture_ptr;
    return nullptr;

}

std::shared_ptr<omp::Texture> omp::MaterialManager::loadTextureLazily(const std::string& path)
{
     if (m_Textures.find(path) != m_Textures.end())
    {
        return m_Textures.at(path);
    }

    auto texture_ptr = std::make_shared<Texture>(path);
    if (!m_VulkanContext.expired())
    {
        texture_ptr->specifyVulkanContext(m_VulkanContext.lock());
    }
    texture_ptr->lazyLoad();
    m_Textures.insert({path, texture_ptr});
    return texture_ptr;
    // TODO: maybe we dont need this
    return nullptr;
}

omp::CubeMapHandle omp::MaterialManager::loadCubeMapTexture(const std::vector<std::string>& inPaths)
{
     TextureConfig config;
    config.type = TextureConfig::CUBEMAP;
    config.layer_amount = inPaths.size();
    auto texture_ptr = std::make_shared<Texture>(m_VulkanContext.lock(), inPaths, config);

    CubeMapHandle handle{};
    m_CubeMaps.insert(std::pair<omp::CubeMapHandle, std::shared_ptr<omp::Texture>>{handle, texture_ptr});

    if (!m_VulkanContext.expired())
    {
        texture_ptr->specifyVulkanContext(m_VulkanContext.lock());
    }
    texture_ptr->lazyLoad();
    return handle;
    // TODO: look into aftern refactor cubemaps
    return CubeMapHandle();
}

std::shared_ptr<omp::Texture> omp::MaterialManager::getTexture(const std::string& path) const
{
    if (m_Textures.find(path) != m_Textures.end())
    {
        return m_Textures.at(path);
    }
    WARN(LogRendering, "Texture do not exists: " + path);
    return nullptr;
}

std::shared_ptr<omp::Material> omp::MaterialManager::createOrGetMaterial(const std::string& path)
{
    if (m_Materials.find(path) != m_Materials.end())
    {
        return m_Materials.at(path);
    }

    auto mat = std::make_shared<omp::Material>(path);
    mat->m_Manager = this;
    m_Materials.insert({path, mat});
    return mat;
}

omp::MaterialManager::MaterialManager()
{
    static const std::string default_path = "../textures/default.png";
    static const std::string empty_path = "../textures/empty.jpg";
    m_DefaultTexture = loadTextureLazily(default_path);
    m_EmptyTexture = loadTextureLazily(empty_path);

    m_Textures.insert({default_path, m_DefaultTexture});
    m_Textures.insert({empty_path, m_EmptyTexture});
}

void omp::MaterialManager::specifyVulkanContext(const std::shared_ptr<omp::VulkanContext>& inContext)
{
    m_VulkanContext = inContext;
    for (auto& [name, texture] : m_Textures)
    {
        if (texture) texture->specifyVulkanContext(inContext);
    }
}

void omp::MaterialManager::clearGpuState() const
{
    for (auto& texture_pair: m_Textures)
    {
        texture_pair.second->destroyVkObjects();
    }
}
*/