#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "Rendering/VulkanContext.h"
#include "Rendering/Texture.h"
#include "Rendering/Material.h"

class Renderer;
namespace omp
{
    struct CubeMapHandle;
}

struct omp::CubeMapHandle
{
    uint32_t id;

    CubeMapHandle()
    {
        static uint32_t static_id = 0;
        id = static_id;
        static_id++;
    }
    CubeMapHandle(const CubeMapHandle& handle)
    {
        id = handle.id;
    }

    bool operator==(const CubeMapHandle& inHandle) const
    {
        return id == inHandle.id;
    }
};

namespace std
{
    template<>
    struct hash<omp::CubeMapHandle>
    {
        size_t operator()(const omp::CubeMapHandle& k) const
        {
            return hash<uint32_t>()(k.id);
        }
    };
}

namespace omp
{
    class MaterialManager
    {
        std::weak_ptr<VulkanContext> m_VulkanContext;

        // TODO: Transit all textures to handles
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
        std::unordered_map<CubeMapHandle, std::shared_ptr<omp::Texture>> m_CubeMaps;
        std::unordered_map<std::string, std::shared_ptr<omp::Material>> m_Materials;

        std::shared_ptr<omp::Texture> m_DefaultTexture;
        std::shared_ptr<omp::Texture> m_EmptyTexture;

        MaterialManager();
        void clearGpuState() const;
        void specifyVulkanContext(const std::shared_ptr<omp::VulkanContext>& inContext);
        std::unordered_map<std::string, std::shared_ptr<omp::Material>>& getMaterials() { return m_Materials; };
    public:
        inline static MaterialManager& getMaterialManager()
        {
            static MaterialManager single;
            return single;
        }

        std::shared_ptr<omp::Texture> loadTextureInstantly(const std::string& path);
        std::shared_ptr<omp::Texture> loadTextureLazily(const std::string& path);
        CubeMapHandle loadCubeMapTexture(const std::vector<std::string>& inPaths);

        std::shared_ptr<omp::Material> createOrGetMaterial(const std::string& path);

        std::shared_ptr<Texture> getTexture(const std::string& path) const;

        std::weak_ptr<omp::Texture> getDefaultTexture() const { return m_DefaultTexture; }

        std::weak_ptr<omp::Texture> getEmptyTexture() const { return m_EmptyTexture; }


        friend class ::Renderer;
    };
} // omp

