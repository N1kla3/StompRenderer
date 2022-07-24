#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "VulkanContext.h"
#include "Texture.h"
#include "Material.h"

namespace omp{
class MaterialManager
{
    std::weak_ptr<VulkanContext> m_VkHelper;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
    std::unordered_map<std::string, std::shared_ptr<omp::Material>> m_Materials;

public:
    explicit MaterialManager(const std::shared_ptr<VulkanContext>& helper);
    ~MaterialManager();

    std::shared_ptr<omp::Texture> LoadTextureInstantly(const std::string& path);
    std::shared_ptr<omp::Texture> LoadTextureLazily(const std::string& path);

    std::shared_ptr<omp::Material> CreateMaterial(const std::string& name);

    std::shared_ptr<Texture> GetTexture(const std::string& path) const;
};
} // omp
