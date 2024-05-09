#include "LightSystem.h"

omp::LightSystem::LightSystem(const std::shared_ptr<omp::VulkanContext>& inVulkanContext, uint32_t khrNum)
    : m_VulkanContext(inVulkanContext)
    , m_KHRnum(khrNum)
{
    tryRecreateBuffers();
}

void omp::LightSystem::tryRecreateBuffers()
{
    if (m_GlobalLight)
    {
        m_GlobalBuffer = std::make_unique<omp::UniformBuffer>(
                m_VulkanContext,
                m_KHRnum,
                sizeof(GlobalLight),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                );
    }
    if (!m_PointLights.empty())
    {
        m_PointBuffer = std::make_unique<omp::UniformBuffer>(
                m_VulkanContext,
                m_KHRnum,
                sizeof(PointLight) * m_PointLights.size(),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                );
    }
    if (!m_SpotLights.empty())
    {
        m_SpotBuffer = std::make_unique<omp::UniformBuffer>(
                m_VulkanContext,
                m_KHRnum,
                sizeof(SpotLight) * m_SpotLights.size(),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                );
    }
}

void omp::LightSystem::update()
{
    if (m_GlobalLight)
    {
        m_GlobalLight->updateLightObject();
    }
    else
    {
        WARN(LogRendering, "Global light is null in light system");
    }
    for (auto& light : m_PointLights)
    {
        light->updateLightObject();
    }
    for (auto& light : m_SpotLights)
    {
        light->updateLightObject();
    }
}

void omp::LightSystem::mapMemory(uint32_t khrImage)
{
    if (m_GlobalBuffer)
    {
        m_GlobalBuffer->mapMemory(m_GlobalLight->getLight(), khrImage);
    }
    else
    {
        WARN(LogRendering, "Buffer for global light is null");
    }

    int offset = 0;
    for (size_t index = 0; index < m_PointLights.size(); index++)
    {
        m_PointBuffer->mapMemory(m_PointLights[index]->getLight(), khrImage, offset);
        offset += sizeof(PointLight);
    }

    offset = 0;
    for (size_t index = 0; index < m_SpotLights.size(); index++)
    {
        m_SpotBuffer->mapMemory(m_SpotLights[index]->getLight(), khrImage, offset);
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

std::shared_ptr<omp::LightObject<omp::GlobalLight>> omp::LightSystem::enableGlobalLight(const std::shared_ptr<omp::ModelInstance>& inModel)
{
    m_GlobalLight = std::make_shared<LightObject<GlobalLight>>("Global Light", inModel);
    // TODO maybe global should not have representation
    return m_GlobalLight;
}

void omp::LightSystem::addPointLight(const std::shared_ptr<LightObject<omp::PointLight>>& inLight)
{
    m_PointLights.push_back(inLight);
}

void omp::LightSystem::addSpotLight(const std::shared_ptr<LightObject<omp::SpotLight>>& inLight)
{
    m_SpotLights.push_back(inLight);
}
