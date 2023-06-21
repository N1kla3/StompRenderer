#include "LightSystem.h"

omp::LightSystem::LightSystem(const std::shared_ptr<omp::VulkanContext>& inVulkanContext, uint32_t khrNum)
    : m_VulkanContext(inVulkanContext)
    , m_KHRnum(khrNum)
{
    recreate();
}

void omp::LightSystem::recreate()
{
    m_GlobalBuffer = std::make_unique<omp::UniformBuffer>(m_VulkanContext, m_KHRnum, sizeof(m_GlobalLight.getLight()));
    m_PointBuffer = std::make_unique<omp::UniformBuffer>(m_VulkanContext, m_KHRnum, sizeof(PointLight) * LIGHT_COUNT);
    m_SpotBuffer = std::make_unique<omp::UniformBuffer>(m_VulkanContext, m_KHRnum, sizeof(SpotLight) * LIGHT_COUNT);
}

void omp::LightSystem::update()
{
    m_GlobalLight.updateLightObject();
    for (auto& light : m_PointLights)
    {
        light.updateLightObject();
    }
    for (auto& light : m_SpotLights)
    {
        light.updateLightObject();
    }
}

void omp::LightSystem::mapMemory(uint32_t khrImage)
{
    m_GlobalBuffer->mapMemory(m_GlobalLight.getLight(), khrImage);

    int offset = 0;
    for (size_t index = 0; index < m_PointLights.size(); index++)
    {
        m_PointBuffer->mapMemory(m_PointLights[index].getLight(), khrImage, offset);
        offset += sizeof(PointLight);
    }

    offset = 0;
    for (size_t index = 0; index < m_SpotLights.size(); index++)
    {
        m_SpotBuffer->mapMemory(m_SpotLights[index].getLight(), khrImage, offset);
        offset += sizeof(SpotLight);
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

void omp::LightSystem::setModelForEach(const std::shared_ptr<omp::Model>& inModel)
{
    m_GlobalLight.setModel(inModel);

    for (auto& point_light : m_PointLights)
    {
        point_light.setModel(inModel);
    }
    for (auto& spot_light : m_SpotLights)
    {
        spot_light.setModel(inModel);
    }
}
