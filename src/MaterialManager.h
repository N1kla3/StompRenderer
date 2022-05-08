#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "VulkanHelper.h"
#include "Texture.h"

namespace omp{
class MaterialManager
{
    std::weak_ptr<VulkanHelper> m_VkHelper;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;

public:
    explicit MaterialManager(const std::shared_ptr<VulkanHelper>& helper);
    ~MaterialManager();

    std::shared_ptr<omp::Texture> LoadTextureInstantly(const std::string& path);
    std::shared_ptr<omp::Texture> LoadTextureLazily(const std::string& path);

    std::shared_ptr<Texture> GetTexture(const std::string& path) const;
};
} // omp
