#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "VulkanContext.h"
#include "Texture.h"
#include "Material.h"

namespace omp
{
    class MaterialManager
    {
        std::weak_ptr<VulkanContext> m_VkHelper;

        std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
        std::unordered_map<std::string, std::shared_ptr<omp::Material>> m_Materials;

        std::shared_ptr<omp::Texture> m_DefaultTexture;
        std::shared_ptr<omp::Texture> m_EmptyTexture;

    public:
        explicit MaterialManager(const std::shared_ptr<VulkanContext>& helper);
        ~MaterialManager();

        std::shared_ptr<omp::Texture> loadTextureInstantly(const std::string& path);
        std::shared_ptr<omp::Texture> loadTextureLazily(const std::string& path);

        std::shared_ptr<omp::Material> createMaterial(const std::string& name);

        std::shared_ptr<Texture> getTexture(const std::string& path) const;

        std::weak_ptr<omp::Texture> getDefaultTexture() const { return m_DefaultTexture; }

        std::weak_ptr<omp::Texture> getEmptyTexture() const { return m_EmptyTexture; }
    };
} // omp
