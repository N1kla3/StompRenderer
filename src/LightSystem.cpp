#include "LightSystem.h"
#include "Light.h"
#include "LightObject.h"
#include "Scene.h"

omp::LightSystem::LightSystem(const std::shared_ptr<omp::VulkanContext>& inVulkanContext, uint32_t khrNum)
    : m_VulkanContext(inVulkanContext)
    , m_KHRnum(khrNum)
{
    tryRecreateBuffers();
}

void omp::LightSystem::tryRecreateBuffers()
{
    m_GlobalBuffer = std::make_unique<omp::UniformBuffer>(
            m_VulkanContext,
            m_KHRnum,
            sizeof(GlobalLight),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
            );

    m_PointBuffer = std::make_unique<omp::UniformBuffer>(
            m_VulkanContext,
            m_KHRnum,
            getPointLightBufferSize(),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

    m_SpotBuffer = std::make_unique<omp::UniformBuffer>(
            m_VulkanContext,
            m_KHRnum,
            getSpotLightBufferSize(),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );
}

void omp::LightSystem::update()
{
    if (!m_CurrentScene) return;

    for (auto& light : m_CurrentScene->getLights())
    {
        light->updateLightObject();
    }
}

void omp::LightSystem::mapMemory(uint32_t khrImage)
{
    if (!m_CurrentScene) return;

    uint32_t offset_p = 0, offset_s = 0;
    for (auto& light : m_CurrentScene->getLights())
    {
        if (light->getType() == omp::ELightType::GLOBAL)
        {
            m_GlobalBuffer->mapMemory(*reinterpret_cast<omp::GlobalLight*>(light->getLight()), khrImage);
        }
        else if (light->getType() == omp::ELightType::POINT)
        {
            m_PointBuffer->mapMemory(*reinterpret_cast<omp::PointLight*>(light->getLight()), khrImage, offset_p);
            offset_p += sizeof(PointLight);
        }
        else if (light->getType() == omp::ELightType::SPOT)
        {
            m_SpotBuffer->mapMemory(*reinterpret_cast<omp::SpotLight*>(light->getLight()), khrImage, offset_s);
            offset_s += sizeof(SpotLight);
        }
    }
}

void omp::LightSystem::onSceneChanged(omp::Scene* scene)
{
    m_CurrentScene = scene;

    m_GlobalLightNum = 0;
    m_PointLightNum = 0;
    m_SpotLightNum = 0;

    if (m_CurrentScene)
    {
        for (auto& light : m_CurrentScene->getLights())
        {
            if (light->getType() == omp::ELightType::GLOBAL)
            {
                m_GlobalLightNum++;
            }
            else if (light->getType() == omp::ELightType::POINT)
            {
                m_PointLightNum++;
            }
            else if (light->getType() == omp::ELightType::SPOT)
            {
                m_SpotLightNum++;
            }
        }
    }
}

VkBuffer omp::LightSystem::getGlobalLightBuffer(uint32_t khr)
{
    if (m_GlobalBuffer)
    {
        return m_GlobalBuffer->getBuffer(khr);
    }
    return 0;
}

VkBuffer omp::LightSystem::getPointLightBuffer(uint32_t khr)
{
    if (m_PointBuffer)
    {
        return m_PointBuffer->getBuffer(khr);
    }
    return 0;
}

VkBuffer omp::LightSystem::getSpotLightBuffer(uint32_t khr)
{
    if (m_SpotBuffer)
    {
        return m_SpotBuffer->getBuffer(khr);
    }
    return 0;
}

