#include "Renderer.h"
#include "imgui.h"
#include <algorithm>
#include <optional>
#include <set>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "IO/stb_image.h"
#include "UI/CameraPanel.h"
#include "UI/EntityPanel.h"
#include "UI/MainLayer.h"
#include "UI/ScenePanel.h"
#include "UI/ViewPort.h"
#include "backends/imgui_impl_glfw.h"

#include <tiny_obj_loader.h>

#include "ImGuizmo/ImGuizmo.h"
#include "Logs.h"
#include "Rendering/ModelStatics.h"

#ifdef NDEBUG
const bool g_EnableValidationLayers = false;
#else
const bool g_EnableValidationLayers = true;
#endif

namespace
{
    const std::vector<const char*> g_ValidationLayers{
            "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char*> g_DeviceExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};
} // namespace

omp::Renderer::Renderer()
{
}

void omp::Renderer::initVulkan(GLFWwindow* window, int initWidth, int initHeight)
{
    m_Window = window;
    m_CurrentWidth = initWidth;
    m_CurrentHeight = initHeight;
    createInstance();
    setupDebugMessenger();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
}

void omp::Renderer::initResources()
{
    createImguiWidgets();
    postSwapChainInitialize();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    m_VulkanContext->setCommandPool(m_CommandPool);
    createColorResources();
    createViewportResources();
    createPickingResources();
    createDepthResources();
    createFramebuffers();
    // TODO: need window, maybe separate imguie resources
    initializeImgui(m_Window);
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    m_CommandBuffers.resize(m_PresentKHRImagesNum);
    m_ImguiCommandBuffers.resize(m_PresentKHRImagesNum);
    createSyncObjects();

}

void omp::Renderer::loadScene(omp::Scene* scene)
{
    m_CurrentScene = scene;

    m_CurrentScene->loadToGPU(m_VulkanContext);

    updateImguiWidgets();
    initializeScene();
    m_LightSystem->onSceneChanged(scene);
    recreateSwapChain();
    // TODO: add request to update pipelines and 
}

void omp::Renderer::requestDrawFrame(float deltaTime)
{
    drawFrame();
    postFrame();
    tick(deltaTime);

    vkDeviceWaitIdle(m_LogicalDevice);
}

void omp::Renderer::cleanup()
{
    vkFreeMemory(m_LogicalDevice, m_PixelReadMemory, nullptr);
    vkDestroyBuffer(m_LogicalDevice, m_PixelReadBuffer, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(m_LogicalDevice, m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_LogicalDevice, m_InFlightFences[i], nullptr);
    }

    cleanupSwapChain();
    
    vkFreeDescriptorSets(m_LogicalDevice, m_DescriptorPool, static_cast<uint32_t>(m_MaterialSets.size()),
                         m_MaterialSets.data());
    m_MaterialSets.clear();

    destroyAllCommandBuffers();
    vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
    vkDestroyCommandPool(m_LogicalDevice, m_ImguiCommandPool, nullptr);

    if (g_EnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }

    m_UboBuffer.reset();
    m_OutlineBuffer.reset();
    m_LightSystem.reset();

    vkDestroyDescriptorSetLayout(m_LogicalDevice, m_UboDescriptorSetLayout,
                                 nullptr);
    vkDestroyDescriptorSetLayout(m_LogicalDevice, m_OutlineSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_LogicalDevice, m_SkyboxSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_LogicalDevice, m_TexturesDescriptorSetLayout,
                                 nullptr);
    vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, nullptr);

    //omp::MaterialManager::getMaterialManager().clearGpuState();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(m_LogicalDevice, m_ImguiDescriptorPool, nullptr);

    vkDestroyDevice(m_LogicalDevice, nullptr);

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

    vkDestroyInstance(m_Instance, nullptr);

}

void omp::Renderer::createInstance()
{
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    if (g_EnableValidationLayers)
    {
        create_info.enabledLayerCount =
                static_cast<uint32_t>(g_ValidationLayers.size());
        create_info.ppEnabledLayerNames = g_ValidationLayers.data();
        populateDebugMessengerCreateInfo(debug_create_info);
        create_info.pNext = &debug_create_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }
    auto required_extensions = getRequiredExtensions();
    create_info.enabledExtensionCount =
            static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();

    VkResult result = vkCreateInstance(&create_info, nullptr, &m_Instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to craete vk instance");
    }

    uint32_t ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

    std::vector<VkExtensionProperties> extensions(ext_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count,
                                           extensions.data());

    INFO(LogRendering, "Available extensions");
    for (const auto& ext: extensions)
    {
        INFO(LogRendering, ext.extensionName);
    }

    if (g_EnableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available");
    }
}

bool omp::Renderer::checkValidationLayerSupport()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    bool layer_found = false;

    for (const char* layer_name: g_ValidationLayers)
    {
        for (const auto& layer_property: available_layers)
        {
            if (strcmp(layer_name, layer_property.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
        {
            return false;
        }
    }
    return true;
}

std::vector<const char*> omp::Renderer::getRequiredExtensions()
{
    uint32_t glfw_extensions_count = 0;
    const char** glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
    std::vector<const char*> extensions(glfw_extensions,
                                        glfw_extensions + glfw_extensions_count);
    if (g_EnableValidationLayers)
    {
        extensions.push_back("VK_EXT_debug_utils");
    }
    return extensions;
}

void omp::Renderer::setupDebugMessenger()
{
    if (!g_EnableValidationLayers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    populateDebugMessengerCreateInfo(create_info);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &create_info, nullptr,
                                     &m_DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to setup debug messenger");
    }
}

void omp::Renderer::populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void omp::Renderer::pickPhysicalDevice()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_Instance, &device_count, nullptr);
    if (device_count == 0)
    {
        throw std::runtime_error("failed to find GPU physical devices");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_Instance, &device_count, devices.data());

    for (const auto& device: devices)
    {
        if (isDeviceSuitable(device))
        {
            m_PhysDevice = device;
            m_MSAASamples = getMaxUsableSampleCount();
            break;
        }
    }
    if (m_PhysDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find suitable gpu");
    }
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(m_PhysDevice, &physical_device_properties);
    m_DeviceLimits = physical_device_properties.limits;
}

bool omp::Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    auto indices = findQueueFamilies(device);

    bool extensions_supported = checkDeviceExtensionSupport(device);

    bool swap_chain_adequate = false;
    if (extensions_supported)
    {
        SwapChainSupportDetails swap_chain_support = querySwapChainSupport(device);
        swap_chain_adequate = !swap_chain_support.formats.empty() &&
                              !swap_chain_support.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           features.geometryShader && indices.IsComplete() &&
           extensions_supported && swap_chain_adequate &&
           supported_features.samplerAnisotropy;
}

omp::Renderer::QueueFamilyIndices
omp::Renderer::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices{};
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             queue_families.data());

    uint32_t i = 0;
    for (const auto& queue: queue_families)
    {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &present_support);
        if (present_support)
        {
            indices.present_family = i;
        }
        if (indices.IsComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

void omp::Renderer::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_PhysDevice);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                                indices.present_family.value()};

    float queue_priority = 1.f;
    for (uint32_t queue_family: unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount =
            static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount =
            static_cast<uint32_t>(g_DeviceExtensions.size());
    create_info.ppEnabledExtensionNames = g_DeviceExtensions.data();

    if (g_EnableValidationLayers)
    {
        create_info.enabledLayerCount =
                static_cast<uint32_t>(g_ValidationLayers.size());
        create_info.ppEnabledLayerNames = g_ValidationLayers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_PhysDevice, &create_info, nullptr, &m_LogicalDevice) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(m_LogicalDevice, indices.graphics_family.value(), 0,
                     &m_GraphicsQueue);
    vkGetDeviceQueue(m_LogicalDevice, indices.present_family.value(), 0,
                     &m_PresentQueue);

    createCommandPool();
    m_VulkanContext = std::make_shared<omp::VulkanContext>(
            m_LogicalDevice, m_PhysDevice, m_CommandPool, m_GraphicsQueue);
    //omp::MaterialManager::getMaterialManager().specifyVulkanContext(
    //        m_VulkanContext);
    m_RenderPass = std::make_shared<omp::RenderPass>(m_LogicalDevice);
    m_ImguiRenderPass = std::make_shared<omp::RenderPass>(m_LogicalDevice);
}

void omp::Renderer::createSurface(GLFWwindow* window)
{
    if (glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}

bool omp::Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         available_extensions.data());

    std::set<std::string> required_extensions(g_DeviceExtensions.begin(),
                                              g_DeviceExtensions.end());

    for (const auto& extension: available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
}

omp::Renderer::SwapChainSupportDetails
omp::Renderer::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface,
                                              &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &format_count,
                                         nullptr);

    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &format_count,
                                             details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface,
                                              &present_mode_count, nullptr);

    if (present_mode_count != 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
                device, m_Surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

VkSurfaceFormatKHR omp::Renderer::chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& available_format: availableFormats)
    {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR omp::Renderer::chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& available_presentation_mode: availablePresentModes)
    {
        if (available_presentation_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_presentation_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
omp::Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR&)
{
    // TODO: this is strange and possibly incorrect
    
    /* if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actual_extent = {WIDTH, HEIGHT};

        actual_extent.width =
                std::max<uint32_t>(capabilities.minImageExtent.width,
                                   std::min<uint32_t>(capabilities.maxImageExtent.width,
                                                      actual_extent.width));

        actual_extent.height = std::max<uint32_t>(
                capabilities.minImageExtent.height,
                std::min<uint32_t>(capabilities.maxImageExtent.height,
                                   actual_extent.height));

        int h, w;
        glfwGetFramebufferSize(m_Window, &w, &h);
        VkExtent2D actual_extent_sec = {static_cast<uint32_t>(w),
                                        static_cast<uint32_t>(h)};
        return actual_extent_sec;
    } */

    VkExtent2D actual_extent_sec = {static_cast<uint32_t>(m_CurrentWidth),
                                    static_cast<uint32_t>(m_CurrentHeight)};
    return actual_extent_sec;
}

void omp::Renderer::createSwapChain()
{
    SwapChainSupportDetails swap_chain_support =
            querySwapChainSupport(m_PhysDevice);

    VkSurfaceFormatKHR surface_format =
            chooseSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode =
            chooseSwapPresentMode(swap_chain_support.present_modes);
    VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

    m_PresentKHRImagesNum = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 &&
        m_PresentKHRImagesNum > swap_chain_support.capabilities.maxImageCount)
    {
        m_PresentKHRImagesNum = swap_chain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_Surface;
    create_info.minImageCount = m_PresentKHRImagesNum;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(m_PhysDevice);
    uint32_t queue_family_indices[] = {indices.graphics_family.value(),
                                       indices.present_family.value()};

    if (indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }
    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_LogicalDevice, &create_info, nullptr,
                             &m_SwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &m_PresentKHRImagesNum,
                            nullptr);
    m_SwapChainImages.resize(m_PresentKHRImagesNum);
    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &m_PresentKHRImagesNum,
                            m_SwapChainImages.data());

    m_SwapChainImageFormat = surface_format.format;
    m_SwapChainExtent = extent;
}

void omp::Renderer::postSwapChainInitialize()
{
    m_LightSystem = std::make_unique<omp::LightSystem>(m_VulkanContext,
                                                       m_PresentKHRImagesNum);

    m_VulkanContext->createBuffer(sizeof(int32_t),
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  m_PixelReadBuffer, m_PixelReadMemory);
}

void omp::Renderer::prepareSceneForRendering()
{
    if (m_CurrentScene)
    {

    }
    else
    {
        WARN(LogRendering, "Cant prepare scene for rendering, scene is empty");
    }
}

void omp::Renderer::createImageViews()
{
    m_SwapChainImageViews.resize(m_PresentKHRImagesNum);
    for (size_t i = 0; i < m_PresentKHRImagesNum; i++)
    {
        m_SwapChainImageViews[i] = m_VulkanContext->createImageView(
                m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT,
                1);
    }
}

void omp::Renderer::createGraphicsPipeline()
{
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor =
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkStencilOpState stencil_state{};
    stencil_state.compareOp = VK_COMPARE_OP_ALWAYS;
    stencil_state.failOp = VK_STENCIL_OP_REPLACE;
    stencil_state.depthFailOp = VK_STENCIL_OP_REPLACE;
    stencil_state.passOp = VK_STENCIL_OP_REPLACE;
    stencil_state.compareMask = 0xff;
    stencil_state.writeMask = 0xff;
    stencil_state.reference = 1;

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = stencil_state;
    depth_stencil.back = stencil_state;

    // Light pipeline
    std::shared_ptr<omp::Shader> light_shader = std::make_shared<omp::Shader>(
            m_VulkanContext, "../SPRV/shaderLightvert.spv",
            "../SPRV/shaderLightfrag.spv");

    std::unique_ptr<omp::GraphicsPipeline> light_pipe =
            std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    light_pipe->startDefaultCreation();
    light_pipe->addColorBlendingAttachment(color_blend_attachment);
    light_pipe->addColorBlendingAttachment(color_blend_attachment);
    light_pipe->createMultisamplingInfo(m_MSAASamples);
    light_pipe->createViewport(m_SwapChainExtent);
    light_pipe->definePushConstant<omp::ModelPushConstant>(
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    light_pipe->addPipelineSetLayout(m_UboDescriptorSetLayout);
    light_pipe->addPipelineSetLayout(m_TexturesDescriptorSetLayout);
    light_pipe->createShaders(light_shader);
    light_pipe->setDepthStencil(depth_stencil);
    light_pipe->confirmCreation(m_RenderPass);

    std::shared_ptr<omp::Shader> skybox_shader = std::make_shared<omp::Shader>(
            m_VulkanContext, "../SPRV/skyboxvert.spv", "../SPRV/skyboxfrag.spv");

    std::unique_ptr<omp::GraphicsPipeline> skybox_pipe =
            std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    skybox_pipe->startDefaultCreation();
    skybox_pipe->addColorBlendingAttachment(color_blend_attachment);
    skybox_pipe->addColorBlendingAttachment(color_blend_attachment);
    skybox_pipe->createMultisamplingInfo(m_MSAASamples);
    skybox_pipe->createViewport(m_SwapChainExtent);
    // ?
    // skybox_pipe->definePushConstant<omp::ModelPushConstant>(VK_SHADER_STAGE_VERTEX_BIT
    // | VK_SHADER_STAGE_FRAGMENT_BIT);
    skybox_pipe->addPipelineSetLayout(m_SkyboxSetLayout);
    // ? skybox_pipe->addPipelineSetLayout(m_TexturesDescriptorSetLayout);
    depth_stencil.depthTestEnable = VK_TRUE;
    skybox_pipe->setDepthStencil(depth_stencil);
    skybox_pipe->createShaders(skybox_shader);
    skybox_pipe->confirmCreation(m_RenderPass);
    depth_stencil.depthTestEnable = VK_FALSE;

    // Simple pipeline
    std::shared_ptr<omp::Shader> shader = std::make_shared<omp::Shader>(
            m_VulkanContext, "../SPRV/shadervert.spv", "../SPRV/shaderfrag.spv");

    std::unique_ptr<omp::GraphicsPipeline> pipe =
            std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    pipe->startDefaultCreation();
    pipe->addColorBlendingAttachment(color_blend_attachment);
    pipe->addColorBlendingAttachment(color_blend_attachment);
    pipe->createMultisamplingInfo(m_MSAASamples);
    pipe->createViewport(m_SwapChainExtent);
    pipe->definePushConstant<omp::ModelPushConstant>(
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    pipe->addPipelineSetLayout(m_UboDescriptorSetLayout);
    pipe->addPipelineSetLayout(m_TexturesDescriptorSetLayout);
    pipe->setDepthStencil(depth_stencil);
    pipe->createShaders(shader);
    pipe->confirmCreation(m_RenderPass);

    std::shared_ptr<omp::Shader> blend_shader = std::make_shared<omp::Shader>(
            m_VulkanContext, "../SPRV/shaderLightBlendvert.spv",
            "../SPRV/shaderLightBlendfrag.spv");
    VkPipelineRasterizationStateCreateInfo rasterization_state{};
    rasterization_state.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.depthClampEnable = VK_FALSE;
    rasterization_state.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state.lineWidth = 1.0f;
    rasterization_state.cullMode = VK_CULL_MODE_NONE;
    rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state.depthBiasEnable = VK_FALSE;
    rasterization_state.depthBiasConstantFactor = 0.0f;
    rasterization_state.depthBiasClamp = 0.0f;
    rasterization_state.depthBiasSlopeFactor = 0.0f;
    std::unique_ptr<omp::GraphicsPipeline> grass_pipe =
            std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    grass_pipe->startDefaultCreation();
    color_blend_attachment.blendEnable = VK_TRUE;
    grass_pipe->addColorBlendingAttachment(color_blend_attachment);
    color_blend_attachment.blendEnable = VK_FALSE;
    grass_pipe->addColorBlendingAttachment(color_blend_attachment);
    grass_pipe->createMultisamplingInfo(m_MSAASamples);
    grass_pipe->createViewport(m_SwapChainExtent);
    grass_pipe->createRasterizer(rasterization_state);
    grass_pipe->definePushConstant<omp::ModelPushConstant>(
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    grass_pipe->addPipelineSetLayout(m_UboDescriptorSetLayout);
    grass_pipe->addPipelineSetLayout(m_TexturesDescriptorSetLayout);
    grass_pipe->setDepthStencil(depth_stencil);
    grass_pipe->createShaders(blend_shader);
    grass_pipe->confirmCreation(m_RenderPass);

    // LIGHT STENCIL
    color_blend_attachment.blendEnable = VK_FALSE;
    std::unique_ptr<omp::GraphicsPipeline> light_stencil =
            std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    light_stencil->startDefaultCreation();
    light_stencil->addColorBlendingAttachment(color_blend_attachment);
    light_stencil->addColorBlendingAttachment(color_blend_attachment);
    light_stencil->createMultisamplingInfo(m_MSAASamples);
    light_stencil->createViewport(m_SwapChainExtent);
    light_stencil->definePushConstant<omp::ModelPushConstant>(
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    light_stencil->addPipelineSetLayout(m_UboDescriptorSetLayout);
    light_stencil->addPipelineSetLayout(m_TexturesDescriptorSetLayout);
    light_stencil->createShaders(light_shader);
    depth_stencil.stencilTestEnable = VK_TRUE;
    light_stencil->setDepthStencil(depth_stencil);
    light_stencil->confirmCreation(m_RenderPass);

    // Outline pipeline
    std::shared_ptr<omp::Shader> outline_shader = std::make_unique<omp::Shader>(
            m_VulkanContext, "../SPRV/outlinevert.spv", "../SPRV/outlinefrag.spv");
    std::unique_ptr<omp::GraphicsPipeline> outline_pipe =
            std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    outline_pipe->startDefaultCreation();
    outline_pipe->addColorBlendingAttachment(color_blend_attachment);
    outline_pipe->addColorBlendingAttachment(color_blend_attachment);
    outline_pipe->createMultisamplingInfo(m_MSAASamples);
    outline_pipe->createViewport(m_SwapChainExtent);
    outline_pipe->addPipelineSetLayout(m_OutlineSetLayout);
    outline_pipe->createShaders(outline_shader);
    stencil_state.compareOp = VK_COMPARE_OP_NOT_EQUAL;
    stencil_state.failOp = VK_STENCIL_OP_KEEP;
    stencil_state.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil_state.passOp = VK_STENCIL_OP_REPLACE;
    depth_stencil.back = stencil_state;
    depth_stencil.front = stencil_state;
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_TRUE;
    outline_pipe->setDepthStencil(depth_stencil);
    outline_pipe->confirmCreation(m_RenderPass);

    m_Pipelines.insert({"Light", std::move(light_pipe)});
    m_Pipelines.insert({"Simple", std::move(pipe)});
    m_Pipelines.insert({"Outline", std::move(outline_pipe)});
    m_Pipelines.insert({"LightStencil", std::move(light_stencil)});
    m_Pipelines.insert({"Grass", std::move(grass_pipe)});
    m_Pipelines.insert({"Skybox", std::move(skybox_pipe)});
}

void omp::Renderer::createRenderPass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format = m_SwapChainImageFormat;
    color_attachment.samples = m_MSAASamples;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription picking_attachment{};
    picking_attachment.format = VK_FORMAT_R32_SINT;
    picking_attachment.samples = m_MSAASamples;
    picking_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    picking_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    picking_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    picking_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    picking_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    picking_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference picking_attach_ref{};
    picking_attach_ref.attachment = 1;
    picking_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = findDepthFormat();
    depth_attachment.samples = m_MSAASamples;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attach_ref{};
    depth_attach_ref.attachment = 2;
    depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = m_SwapChainImageFormat;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 3;
    color_attachment_resolve_ref.layout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription pick_resolve{};
    pick_resolve.format = VK_FORMAT_R32_SINT;
    pick_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    pick_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pick_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    pick_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pick_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    pick_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pick_resolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference pick_resolve_ref{};
    pick_resolve_ref.attachment = 4;
    pick_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentReference, 2> refs{color_attachment_ref,
                                              picking_attach_ref};
    std::array<VkAttachmentReference, 2> resolve_refs{
            color_attachment_resolve_ref, pick_resolve_ref};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(refs.size());
    subpass.pColorAttachments = refs.data();
    subpass.pDepthStencilAttachment = &depth_attach_ref;
    subpass.pResolveAttachments = resolve_refs.data();

    // Use subpass dependencies for layout transitions
    VkSubpassDependency dependencies[2]{};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    m_RenderPass->startConfiguration();

    m_RenderPass->addAttachment(std::move(color_attachment));
    m_RenderPass->addAttachment(std::move(picking_attachment));
    m_RenderPass->addAttachment(std::move(depth_attachment));
    m_RenderPass->addAttachment(std::move(color_attachment_resolve));
    m_RenderPass->addAttachment(std::move(pick_resolve));

    // reference leak if render pass not created in this method
    m_RenderPass->addSubpass(std::move(subpass));
    m_RenderPass->addDependency(std::move(dependencies[0]));
    m_RenderPass->addDependency(std::move(dependencies[1]));

    m_RenderPass->endConfiguration();
}

void omp::Renderer::createFramebuffers()
{
    m_SwapChainFramebuffers.resize(m_PresentKHRImagesNum);

    for (size_t i = 0; i < m_PresentKHRImagesNum; i++)
    {
        createFramebufferAtImage(i);
    }
}

void omp::Renderer::createFramebufferAtImage(size_t index)
{
    std::vector<VkImageView> attachments{m_ColorImageView, m_PickingImageView,
                                         m_DepthImageView, m_ViewportImageView,
                                         m_PickingResolveView};
    omp::FrameBuffer frame_buffer(
            m_LogicalDevice, attachments, m_RenderPass,
            static_cast<uint32_t>(m_RenderViewport->getSize().x),
            static_cast<uint32_t>(m_RenderViewport->getSize().y));
    m_SwapChainFramebuffers[index] = frame_buffer;

    // test
    INFO(LogRendering, "Framebuffer created with size {} {}", m_RenderViewport->getSize().x, m_RenderViewport->getSize().y);
}

void omp::Renderer::createCommandPool()
{
    QueueFamilyIndices queue_family_indices = findQueueFamilies(m_PhysDevice);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr,
                            &m_CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool");
    }
}

void omp::Renderer::prepareFrameForImage(size_t KHRImageIndex)
{
    prepareCommandBuffer(m_ImguiCommandBuffers[KHRImageIndex],
                         m_ImguiCommandPool);

    std::vector<VkClearValue> clear_value{1};
    clear_value[0].color = g_ClearColor;

    VkRect2D rect{};
    // Main Render pass
    VkCommandBuffer& main_buffer = m_CommandBuffers[KHRImageIndex].buffer;
    prepareCommandBuffer(m_CommandBuffers[KHRImageIndex], m_CommandPool);

    std::vector<VkClearValue> clear_values{3};
    clear_values[0].color = g_ClearColor;
    clear_values[1].color = g_ClearColor;
    clear_values[2].depthStencil = {1.0f, 0};

    rect.offset.x = 0;
    rect.offset.y = 0;
    rect.extent.height = static_cast<uint32_t>(m_RenderViewport->getSize().y);
    rect.extent.width = static_cast<uint32_t>(m_RenderViewport->getSize().x);
    beginRenderPass(m_RenderPass.get(), main_buffer,
                    m_SwapChainFramebuffers[KHRImageIndex], clear_values, rect);
    setViewport(main_buffer);

    omp::SceneEntity* outline_entity = nullptr;
    VkDeviceSize offsets[] = {0};

    std::span<std::unique_ptr<omp::SceneEntity>> scene_ref =
            m_CurrentScene->getEntities();
    std::sort(
            scene_ref.begin(), scene_ref.end(),
            [this](
                    const std::unique_ptr<omp::SceneEntity>& inEnt,
                    const std::unique_ptr<omp::SceneEntity>& inEnt2) -> bool
            {
                if (inEnt->getModelInstance()
                            ->getMaterialInstance()
                            ->getStaticMaterial()
                            .lock()
                            ->isBlendingEnabled() &&
                    !inEnt2->getModelInstance()
                            ->getMaterialInstance()
                            ->getStaticMaterial()
                            .lock()
                            ->isBlendingEnabled())
                {
                    return false;
                }
                if (!inEnt->getModelInstance()
                        ->getMaterialInstance()
                        ->getStaticMaterial()
                        .lock()
                        ->isBlendingEnabled() &&
                    inEnt2->getModelInstance()
                            ->getMaterialInstance()
                            ->getStaticMaterial()
                            .lock()
                            ->isBlendingEnabled())
                {
                    return true;
                }
                // TODO remove sqrt
                return glm::distance(m_CurrentScene->getCurrentCamera()->getPosition(),
                                     inEnt->getModelInstance()->getPosition()) >
                       glm::distance(m_CurrentScene->getCurrentCamera()->getPosition(),
                                     inEnt2->getModelInstance()->getPosition());
                // return true;
            });
    for (size_t index = 0; index < scene_ref.size(); index++)
    {
        auto& scene_entity = scene_ref[index];
        auto& material_instance = scene_entity->getModelInstance()->getMaterialInstance();
        auto material = material_instance->getStaticMaterial().lock();
        if (!material)
        {
            WARN(LogRendering, "Material is invalid in material instance");
        }

        VkPipeline model_pipeline{};
        VkPipelineLayout model_pipeline_layout{};
        if (scene_entity->getId() == m_CurrentScene->getCurrentId())
        {
            // TODO check this for valid shader, because light have simple shader, and
            // should not have lightstencil layouts
            outline_entity = scene_entity.get();
            model_pipeline =
                    findGraphicsPipeline("LightStencil")->getGraphicsPipeline();
            model_pipeline_layout =
                    findGraphicsPipeline("LightStencil")->getPipelineLayout();
        }
        else
        {
            model_pipeline = findGraphicsPipeline(material->getShaderName())
                    ->getGraphicsPipeline();
            model_pipeline_layout =
                    findGraphicsPipeline(material->getShaderName())->getPipelineLayout();
        }
        vkCmdBindPipeline(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          model_pipeline);

        vkCmdBindDescriptorSets(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                model_pipeline_layout, 0, 1,
                                &m_UboDescriptorSets[KHRImageIndex], 0, nullptr);

        vkCmdBindVertexBuffers(
                main_buffer, 0, 1,
                &scene_entity->getModelInstance()->getModel().lock()->getVertexBuffer(),
                offsets);
        vkCmdBindIndexBuffer(
                main_buffer,
                scene_entity->getModelInstance()->getModel().lock()->getIndexBuffer(), 0,
                VK_INDEX_TYPE_UINT32);

        omp::ModelPushConstant constant{
                scene_entity->getModelInstance()->getTransform(),
                material_instance->getAmbient(), material_instance->getDiffusive(),
                material_instance->getSpecular(), scene_entity->getId()};
        vkCmdPushConstants(main_buffer, model_pipeline_layout,
                           VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(omp::ModelPushConstant), &constant);

        // TODO: MATERIALS ARE TOTAL SHIT
        if (material)
        {
            retrieveMaterialRenderState(material);
            vkCmdBindDescriptorSets(
                    main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model_pipeline_layout,
                    1, 1, &material->getDescriptorSet()[KHRImageIndex], 0, nullptr);
        }
        else
        {
            // default material
            vkCmdBindDescriptorSets(
                    main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model_pipeline_layout,
                    1, 1, &m_DefaultMaterial->getDescriptorSet()[KHRImageIndex], 0,
                    nullptr);
        }
        vkCmdDrawIndexed(
                main_buffer,
                static_cast<uint32_t>(
                        scene_entity->getModelInstance()->getModel().lock()->getIndices().size()),
                1, 0, 0, 0);
    }

    if (outline_entity)
    {
        auto outline_pipeline = findGraphicsPipeline("Outline");
        vkCmdBindVertexBuffers(
                main_buffer, 0, 1,
                &outline_entity->getModelInstance()->getModel().lock()->getVertexBuffer(),
                offsets);
        vkCmdBindIndexBuffer(
                main_buffer,
                outline_entity->getModelInstance()->getModel().lock()->getIndexBuffer(), 0,
                VK_INDEX_TYPE_UINT32);
        vkCmdBindPipeline(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          outline_pipeline->getGraphicsPipeline());
        vkCmdBindDescriptorSets(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                outline_pipeline->getPipelineLayout(), 0, 1,
                                &m_OutlineDescriptorSets[KHRImageIndex], 0,
                                nullptr);
        vkCmdDrawIndexed(
                main_buffer,
                static_cast<uint32_t>(
                        outline_entity->getModelInstance()->getModel().lock()->getIndices().size()),
                1, 0, 0, 0);
    }

    endRenderPass(m_RenderPass.get(), main_buffer);

    // UI RENDERPASS
    rect.extent.height = m_SwapChainExtent.height;
    rect.extent.width = m_SwapChainExtent.width;
    rect.offset.x = 0;
    rect.offset.y = 0;
    beginRenderPass(m_ImguiRenderPass.get(),
                    m_ImguiCommandBuffers[KHRImageIndex].buffer,
                    m_ImguiFramebuffers[KHRImageIndex], clear_value, rect);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    renderAllUi();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                    m_ImguiCommandBuffers[KHRImageIndex].buffer);

    endRenderPass(m_ImguiRenderPass.get(),
                  m_ImguiCommandBuffers[KHRImageIndex].buffer);
}

void omp::Renderer::drawFrame()
{
    vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame],
                    VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
            m_LogicalDevice, m_SwapChain, UINT64_MAX,
            m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || m_FramebufferResized)
    {
        recreateSwapChain();
        onViewportResize(image_index);
        m_FramebufferResized = false;
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    if (m_ImagesInFlight[image_index] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_LogicalDevice, 1, &m_ImagesInFlight[image_index], VK_TRUE,
                        UINT64_MAX);
    }
    m_ImagesInFlight[image_index] = m_InFlightFences[m_CurrentFrame];

    onViewportResize(image_index);
    updateUniformBuffer(image_index);
    prepareFrameForImage(image_index);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    std::array<VkCommandBuffer, 2> command_buffers{
            m_CommandBuffers[image_index].buffer,
            m_ImguiCommandBuffers[image_index].buffer};
    submit_info.commandBufferCount =
            static_cast<uint32_t>(command_buffers.size());
    submit_info.pCommandBuffers = command_buffers.data();

    VkSemaphore signal_semaphores[] = {
            m_RenderFinishedSemaphores[m_CurrentFrame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submit_info,
                      m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = {m_SwapChain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    result = vkQueuePresentKHR(m_PresentQueue, &present_info);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }
    vkQueueWaitIdle(m_PresentQueue);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void omp::Renderer::createSyncObjects()
{
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_ImagesInFlight.resize(m_PresentKHRImagesNum, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if ((vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr,
                               &m_ImageAvailableSemaphores[i]) != VK_SUCCESS) ||
            (vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr,
                               &m_RenderFinishedSemaphores[i]) != VK_SUCCESS) ||
            (vkCreateFence(m_LogicalDevice, &fence_info, nullptr,
                           &m_InFlightFences[i]) != VK_SUCCESS))
        {
            throw std::runtime_error("failed to create sync objects for a frame");
        }
    }
}

void omp::Renderer::recreateSwapChain()
{
    vkDeviceWaitIdle(m_LogicalDevice);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createColorResources();
    createViewportResources();
    createPickingResources();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorSets();

    if (m_ViewportDescriptor != VK_NULL_HANDLE)
    {
        ImGui_ImplVulkan_RemoveTexture(m_ViewportDescriptor);
    }
    m_ViewportDescriptor =
            ImGui_ImplVulkan_AddTexture(m_ViewportSampler, m_ViewportImageView,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_RenderViewport->setImageId(reinterpret_cast<ImTextureID>(m_ViewportDescriptor));

    createImguiRenderPass();
    createImguiFramebuffers();

    ImGui_ImplVulkan_SetMinImageCount(2);
}

void omp::Renderer::cleanupSwapChain()
{
    destroyMainRenderPassResources();

    m_Pipelines.clear();
    m_RenderPass->destroyInnerState();

    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
    {
        vkDestroyImageView(m_LogicalDevice, m_SwapChainImageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);

    /* Unique ptr will destory on recreate
    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        vkDestroyBuffer(m_LogicalDevice, m_UniformBuffers[i], nullptr);
        vkFreeMemory(m_LogicalDevice, m_UniformBuffersMemory[i], nullptr);
    }

    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        vkDestroyBuffer(m_LogicalDevice, m_LightBuffer[i], nullptr);
        vkFreeMemory(m_LogicalDevice, m_LightBufferMemory[i], nullptr);
    }
     */

    for (auto& framebuffer: m_ImguiFramebuffers)
    {
        framebuffer.destroyInnerState();
    }

    /* for (auto& [u, material]:
            omp::MaterialManager::getMaterialManager().getMaterials())
    {
        material->clearDescriptorSets();
    } */

    vkFreeDescriptorSets(m_LogicalDevice, m_DescriptorPool,
                         static_cast<uint32_t>(m_UboDescriptorSets.size()), m_UboDescriptorSets.data());
    vkFreeDescriptorSets(m_LogicalDevice, m_DescriptorPool,
                         static_cast<uint32_t>(m_OutlineDescriptorSets.size()),
                         m_OutlineDescriptorSets.data());

    m_ImguiRenderPass->destroyInnerState();
}

void omp::Renderer::onWindowResize(
        int width,
        int height)
{
    m_CurrentWidth = width;
    m_CurrentHeight = height;
    m_FramebufferResized = true;
}

void omp::Renderer::createDescriptorSetLayout()
{
    // TODO: need abstraction
    // Skybox layout
    {
        VkDescriptorSetLayoutBinding skybox_layout_binding{};
        skybox_layout_binding.binding = 0;
        skybox_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        skybox_layout_binding.descriptorCount = 1;
        skybox_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        skybox_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding cubemap_layout{};
        cubemap_layout.binding = 1;
        cubemap_layout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cubemap_layout.descriptorCount = 1;
        cubemap_layout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        cubemap_layout.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
                skybox_layout_binding, cubemap_layout};
        VkDescriptorSetLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = static_cast<uint32_t>(bindings.size());
        info.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_LogicalDevice, &info, nullptr,
                                        &m_SkyboxSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Cant create skybox set layout");
        }
    }

    // OUTLINE LAYOUT
    {
        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = 0;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ubo_layout_binding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 1> ubo_bindings = {
                ubo_layout_binding,
        };

        VkDescriptorSetLayoutCreateInfo ubo_layout_info{};
        ubo_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ubo_layout_info.bindingCount = static_cast<uint32_t>(ubo_bindings.size());
        ubo_layout_info.pBindings = ubo_bindings.data();

        if (vkCreateDescriptorSetLayout(m_LogicalDevice, &ubo_layout_info, nullptr,
                                        &m_OutlineSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error(
                    "Failed to create outline descriptor set layout!");
        }
    }

    // UBO LAYOUT
    {
        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = 0;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        ubo_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding global_light_layout_binding{};
        global_light_layout_binding.binding = 1;
        global_light_layout_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        global_light_layout_binding.descriptorCount = 1;
        global_light_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        global_light_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding point_light_layout_binding{};
        point_light_layout_binding.binding = 2;
        point_light_layout_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        point_light_layout_binding.descriptorCount = 1;
        point_light_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        point_light_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding spot_light_layout_binding{};
        spot_light_layout_binding.binding = 3;
        spot_light_layout_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        spot_light_layout_binding.descriptorCount = 1;
        spot_light_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        spot_light_layout_binding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 4> ubo_bindings = {
                ubo_layout_binding, global_light_layout_binding,
                point_light_layout_binding, spot_light_layout_binding};

        VkDescriptorSetLayoutCreateInfo ubo_layout_info{};
        ubo_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ubo_layout_info.bindingCount = static_cast<uint32_t>(ubo_bindings.size());
        ubo_layout_info.pBindings = ubo_bindings.data();

        if (vkCreateDescriptorSetLayout(m_LogicalDevice, &ubo_layout_info, nullptr,
                                        &m_UboDescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ubo descriptor set layout!");
        }
    }

    // TEXTURES LAYOUT
    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 0;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding diffusive_layout_binding{};
    diffusive_layout_binding.binding = 1;
    diffusive_layout_binding.descriptorCount = 1;
    diffusive_layout_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    diffusive_layout_binding.pImmutableSamplers = nullptr;
    diffusive_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding specular_layout_binding{};
    specular_layout_binding.binding = 2;
    specular_layout_binding.descriptorCount = 1;
    specular_layout_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    specular_layout_binding.pImmutableSamplers = nullptr;
    specular_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> texture_bindings = {
            sampler_layout_binding, diffusive_layout_binding,
            specular_layout_binding};

    VkDescriptorSetLayoutCreateInfo texture_layout_info{};
    texture_layout_info.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    texture_layout_info.bindingCount =
            static_cast<uint32_t>(texture_bindings.size());
    texture_layout_info.pBindings = texture_bindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &texture_layout_info,
                                    nullptr, &m_TexturesDescriptorSetLayout) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture descriptor set layout!");
    }
}

void omp::Renderer::createUniformBuffers()
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    m_UboBuffer = std::make_unique<omp::UniformBuffer>(
            m_VulkanContext, m_PresentKHRImagesNum, buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    m_OutlineBuffer = std::make_unique<omp::UniformBuffer>(
            m_VulkanContext, m_PresentKHRImagesNum, sizeof(OutlineUniformBuffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    m_LightSystem->tryRecreateBuffers();
}

void omp::Renderer::updateUniformBuffer(uint32_t currentImage)
{
    UniformBufferObject ubo{};
    ubo.view = m_CurrentScene->getCurrentCamera()->getViewMatrix();
    ubo.proj = glm::perspective(
            glm::radians(m_CurrentScene->getCurrentCamera()->getViewAngle()),
            static_cast<float>(m_RenderViewport->getSize().x) /
            static_cast<float>(m_RenderViewport->getSize().y),
            m_CurrentScene->getCurrentCamera()->getNearClipping(),
            m_CurrentScene->getCurrentCamera()->getFarClipping());
    ubo.proj[1][1] *= -1;
    ubo.view_position = m_CurrentScene->getCurrentCamera()->getPosition();
    ubo.global_light_enabled = m_LightSystem->getGlobalLightSize() > 0;
    ubo.point_light_size = static_cast<uint32_t>(m_LightSystem->getPointLightSize());
    ubo.spot_light_size = static_cast<uint32_t>(m_LightSystem->getSpotLightSize());
    m_UboBuffer->mapMemory(ubo, currentImage);

    OutlineUniformBuffer outline_buffer{};
    outline_buffer.projection = ubo.proj;
    auto entity = m_CurrentScene->getEntity(m_CurrentScene->getCurrentId());
    if (entity)
    {
        outline_buffer.model =
                glm::scale(entity->getModelInstance()->getTransform(), glm::vec3{1.2f});
    }
    outline_buffer.view = m_CurrentScene->getCurrentCamera()->getViewMatrix();
    m_OutlineBuffer->mapMemory(outline_buffer, currentImage);

    m_LightSystem->update();
    m_LightSystem->mapMemory(currentImage);
}

void omp::Renderer::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // TODO think about size
    pool_sizes[0].descriptorCount = 100;

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = 100;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = 1000;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr,
                               &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void omp::Renderer::createDescriptorSets()
{
    // Skybox
    {
        std::vector<VkDescriptorSetLayout> layouts(m_PresentKHRImagesNum,
                                                   m_SkyboxSetLayout);

        VkDescriptorSetAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool = m_DescriptorPool;
        allocate_info.descriptorSetCount = m_PresentKHRImagesNum;
        allocate_info.pSetLayouts = layouts.data();

        m_SkyboxDescriptorSets.resize(m_PresentKHRImagesNum);
        if (vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info,
                                     m_SkyboxDescriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
        for (uint32_t i = 0; i < m_PresentKHRImagesNum; i++)
        {
            // Same ubo for outline, so use the same
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = m_OutlineBuffer->getBuffer(i);
            buffer_info.offset = 0;
            buffer_info.range = sizeof(OutlineUniformBuffer);

            std::array<VkWriteDescriptorSet, 1> descriptor_writes{};
            descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[0].dstSet = m_SkyboxDescriptorSets[i];
            descriptor_writes[0].dstBinding = 0;
            descriptor_writes[0].dstArrayElement = 0;
            descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[0].descriptorCount = 1;
            descriptor_writes[0].pBufferInfo = &buffer_info;
            vkUpdateDescriptorSets(m_LogicalDevice,
                                   static_cast<uint32_t>(descriptor_writes.size()),
                                   descriptor_writes.data(), 0, nullptr);
        }
    }

    // OUTLINE
    {
        std::vector<VkDescriptorSetLayout> layouts(m_PresentKHRImagesNum,
                                                   m_OutlineSetLayout);

        VkDescriptorSetAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool = m_DescriptorPool;
        allocate_info.descriptorSetCount = m_PresentKHRImagesNum;
        allocate_info.pSetLayouts = layouts.data();

        m_OutlineDescriptorSets.resize(m_PresentKHRImagesNum);
        if (vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info,
                                     m_OutlineDescriptorSets.data()) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
        for (uint32_t i = 0; i < m_PresentKHRImagesNum; i++)
        {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = m_OutlineBuffer->getBuffer(i);
            buffer_info.offset = 0;
            buffer_info.range = sizeof(OutlineUniformBuffer);

            std::array<VkWriteDescriptorSet, 1> descriptor_writes{};
            descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[0].dstSet = m_OutlineDescriptorSets[i];
            descriptor_writes[0].dstBinding = 0;
            descriptor_writes[0].dstArrayElement = 0;
            descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[0].descriptorCount = 1;
            descriptor_writes[0].pBufferInfo = &buffer_info;
            vkUpdateDescriptorSets(m_LogicalDevice,
                                   static_cast<uint32_t>(descriptor_writes.size()),
                                   descriptor_writes.data(), 0, nullptr);
        }
    }
    // UBO
    std::vector<VkDescriptorSetLayout> layouts(m_PresentKHRImagesNum,
                                               m_UboDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = m_DescriptorPool;
    allocate_info.descriptorSetCount = m_PresentKHRImagesNum;
    allocate_info.pSetLayouts = layouts.data();

    m_UboDescriptorSets.resize(m_PresentKHRImagesNum);
    if (vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info,
                                 m_UboDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (uint32_t i = 0; i < m_PresentKHRImagesNum; i++)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = m_UboBuffer->getBuffer(i);
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo global_light_info{};
        global_light_info.buffer = m_LightSystem->getGlobalLightBuffer(i);
        global_light_info.offset = 0;
        global_light_info.range = m_LightSystem->getGlobalLightBufferSize();

        VkDescriptorBufferInfo point_light_info{};
        point_light_info.buffer = m_LightSystem->getPointLightBuffer(i);
        point_light_info.offset = 0;
        point_light_info.range = m_LightSystem->getPointLightBufferSize();

        VkDescriptorBufferInfo spot_light_info{};
        spot_light_info.buffer = m_LightSystem->getSpotLightBuffer(i);
        spot_light_info.offset = 0;
        spot_light_info.range = m_LightSystem->getSpotLightBufferSize();

        std::array<VkWriteDescriptorSet, 4> descriptor_writes{};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = m_UboDescriptorSets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = m_UboDescriptorSets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = &global_light_info;

        descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet = m_UboDescriptorSets[i];
        descriptor_writes[2].dstBinding = 2;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pBufferInfo = &point_light_info;

        descriptor_writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[3].dstSet = m_UboDescriptorSets[i];
        descriptor_writes[3].dstBinding = 3;
        descriptor_writes[3].dstArrayElement = 0;
        descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes[3].descriptorCount = 1;
        descriptor_writes[3].pBufferInfo = &spot_light_info;

        vkUpdateDescriptorSets(m_LogicalDevice,
                               static_cast<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(), 0, nullptr);
    }
}

void omp::Renderer::retrieveMaterialRenderState(
        const std::shared_ptr<omp::Material>& material)
{
    if (!material)
    {
        VWARN(LogRendering, "Material is invalid");
        return;
    }
    if (material->isPotentiallyReadyForRendering())
    {
        return;
    }

    std::vector<VkDescriptorSet> ds;
    std::vector<VkDescriptorSetLayout> layouts(m_PresentKHRImagesNum,
                                               m_TexturesDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = m_DescriptorPool;
    allocate_info.descriptorSetCount = m_PresentKHRImagesNum;
    allocate_info.pSetLayouts = layouts.data();

    ds.resize(m_PresentKHRImagesNum);
    auto err =
            vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info, ds.data());
    if (err != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    const omp::MaterialRenderInfo* const material_render_info =
            material->getRenderInfo();

    // TODO performance
    for (size_t index = 0; index < ds.size(); index++)
    {
        std::vector<VkWriteDescriptorSet> descriptor_writes{};
        descriptor_writes.reserve(ds.size());

        std::vector<VkDescriptorImageInfo> image_infos;
        image_infos.reserve(material_render_info->textures.size());

        for (auto& data: material_render_info->textures)
        {
            VkDescriptorImageInfo image_info{};
            auto& texture = data.texture;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = texture->getImageView();
            image_info.sampler = texture->getSampler();
            image_infos.push_back(image_info);

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = ds[index];
            descriptor_write.dstBinding = data.binding_index;
            descriptor_write.dstArrayElement = 0;
            descriptor_write.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pImageInfo = &image_infos.at(data.binding_index);

            descriptor_writes.push_back(descriptor_write);
        }

        vkUpdateDescriptorSets(m_LogicalDevice,
                               static_cast<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(), 0, nullptr);
    }
    m_MaterialSets.insert(m_MaterialSets.begin(), ds.begin(), ds.end());

    material->setDescriptorSet(ds);
}

VkCommandBuffer omp::Renderer::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = m_CommandPool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_LogicalDevice, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void omp::Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue);
    vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
}

void omp::Renderer::createDepthResources()
{
    VkFormat depth_format = findDepthFormat();
    m_VulkanContext->createImage(
            static_cast<uint32_t>(m_RenderViewport->getSize().x),
            static_cast<uint32_t>(m_RenderViewport->getSize().y), 1, depth_format,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory,
            m_MSAASamples);
    m_DepthImageView = m_VulkanContext->createImageView(
            m_DepthImage, depth_format,
            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 1);

    m_VulkanContext->transitionImageLayout(
            m_DepthImage, depth_format, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkFormat omp::Renderer::findSupportedFormat(
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features)
{
    for (VkFormat format: candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_PhysDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                 (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format!");
}

VkFormat omp::Renderer::findDepthFormat()
{
    return findSupportedFormat(
            {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT,
             VK_FORMAT_D32_SFLOAT},
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void omp::Renderer::createImguiContext()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.Fonts->Build();
    ImGui::StyleColorsDark();

    if (!std::filesystem::exists("imgui.ini"))
    {
        // Use default layout first time 
        ImGui::LoadIniSettingsFromDisk("../default_imgui.ini");
        ImGui::SaveIniSettingsToDisk("imgui.ini");
    }
}

void omp::Renderer::initializeImgui(GLFWwindow* window)
{
    createImguiContext();
    createImguiRenderPass();

    VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}};
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr,
                           &m_ImguiDescriptorPool);

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_Instance;
    init_info.PhysicalDevice = m_PhysDevice;
    init_info.Device = m_LogicalDevice;
    init_info.QueueFamily =
            findQueueFamilies(m_PhysDevice).graphics_family.value();
    init_info.Queue = m_GraphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_ImguiDescriptorPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = m_PresentKHRImagesNum;

    // Imgui render pass should be created before call of this method
    ImGui_ImplVulkan_Init(&init_info, m_ImguiRenderPass->getRenderPass());

    ImGui_ImplVulkan_CreateFontsTexture();

    createImguiFramebuffers();
    createImguiCommandPools();
}

void omp::Renderer::createImguiRenderPass()
{
    VkAttachmentDescription attachment{};
    attachment.format = m_SwapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment{};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_ImguiRenderPass->startConfiguration();
    m_ImguiRenderPass->addAttachment(std::move(attachment));
    m_ImguiRenderPass->addSubpass(std::move(subpass));
    m_ImguiRenderPass->addDependency(std::move(dependency));
    m_ImguiRenderPass->endConfiguration();
}

void omp::Renderer::createImguiCommandPools()
{
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex =
            findQueueFamilies(m_PhysDevice).graphics_family.value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr,
                            &m_ImguiCommandPool))
    {
        throw std::runtime_error("Failed to create imgui command pool");
    }
}

void omp::Renderer::createImguiFramebuffers()
{
    m_ImguiFramebuffers.resize(m_PresentKHRImagesNum);
    for (size_t i = 0; i < m_PresentKHRImagesNum; i++)
    {
        omp::FrameBuffer frame_buffer(m_LogicalDevice, {m_SwapChainImageViews[i]},
                                      m_ImguiRenderPass, m_SwapChainExtent.width,
                                      m_SwapChainExtent.height);
        m_ImguiFramebuffers[i] = frame_buffer;
    }
}

VkSampleCountFlagBits omp::Renderer::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(m_PhysDevice, &physical_device_properties);

    VkSampleCountFlags counts =
            physical_device_properties.limits.framebufferColorSampleCounts &
            physical_device_properties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

void omp::Renderer::createColorResources()
{
    VkFormat color_format = m_SwapChainImageFormat;

    m_VulkanContext->createImage(
            static_cast<uint32_t>(m_RenderViewport->getSize().x),
            static_cast<uint32_t>(m_RenderViewport->getSize().y), 1, color_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ColorImage, m_ColorImageMemory,
            m_MSAASamples);
    m_ColorImageView = m_VulkanContext->createImageView(
            m_ColorImage, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void omp::Renderer::createViewportResources()
{
    VkFormat color_format = m_SwapChainImageFormat;

    m_VulkanContext->createImage(
            static_cast<uint32_t>(m_RenderViewport->getSize().x),
            static_cast<uint32_t>(m_RenderViewport->getSize().y), 1, color_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ViewportImage,
            m_ViewportImageMemory, VK_SAMPLE_COUNT_1_BIT);
    m_ViewportImageView = m_VulkanContext->createImageView(
            m_ViewportImage, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = sampler_info.addressModeU;
    sampler_info.addressModeW = sampler_info.addressModeU;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    if (vkCreateSampler(m_LogicalDevice, &sampler_info, nullptr,
                        &m_ViewportSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create viewport sampler");
    }
}

void omp::Renderer::createPickingResources()
{
    VkFormat image_format = VK_FORMAT_R32_SINT;

    m_VulkanContext->createImage(
            static_cast<uint32_t>(m_RenderViewport->getSize().x),
            static_cast<uint32_t>(m_RenderViewport->getSize().y), 1, image_format,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_PickingImage, m_PickingMemory,
            m_MSAASamples);
    m_PickingImageView = m_VulkanContext->createImageView(
            m_PickingImage, image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    m_VulkanContext->createImage(
            static_cast<uint32_t>(m_RenderViewport->getSize().x),
            static_cast<uint32_t>(m_RenderViewport->getSize().y), 1, image_format,
            VK_IMAGE_TILING_LINEAR,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_PickingResolve,
            m_PickingResolveMemory, VK_SAMPLE_COUNT_1_BIT);
    m_PickingResolveView = m_VulkanContext->createImageView(
            m_PickingResolve, image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void omp::Renderer::renderAllUi()
{
    static auto prev_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
            current_time - prev_time)
            .count();
    prev_time = current_time;

    for (auto& unit: m_Widgets)
    {
        unit->renderUi(time);
    }

    // TODO remove
    /* ImGui::Begin("Imagessss");
    if (omp::MaterialManager::getMaterialManager().getTexture(
            "../textures/viking.png"))
    {
        ImGui::Image((ImTextureID) (omp::MaterialManager::getMaterialManager()
                             .getTexture("../textures/viking.png")
                             ->getTextureId()),
                     {100, 100});
    }
    ImGui::End();*/

    ImGui::ShowDemoWindow();
}

void omp::Renderer::createImguiWidgets()
{
    // ORDER IS IMPORTANT DOCK NODES GO FIRST

    m_RenderViewport = std::make_shared<omp::ViewPort>();

    auto material_panel = std::make_shared<omp::MaterialPanel>();
    m_ScenePanel = std::make_shared<omp::ScenePanel>(material_panel);
    m_LightPanel = std::make_shared<omp::GlobalLightPanel>();

    m_Widgets.push_back(std::make_shared<omp::MainLayer>());
    m_Widgets.push_back(m_RenderViewport);
    m_Widgets.push_back(std::move(material_panel));
    m_Widgets.push_back(m_ScenePanel);
    m_Widgets.push_back(m_LightPanel);

    updateImguiWidgets();
}

void omp::Renderer::updateImguiWidgets()
{
    if (m_CurrentScene)
    {
        m_RenderViewport->setCamera(m_CurrentScene->getCurrentCamera());
        m_ScenePanel->setScene(m_CurrentScene);
        m_LightPanel->setLightRef(nullptr);
    }
    else
    {
        m_RenderViewport->setCamera(nullptr);
        m_ScenePanel->setScene(nullptr);
        m_LightPanel->setLightRef(nullptr);
    }
}


void omp::Renderer::destroyMainRenderPassResources()
{
    vkDestroyImageView(m_LogicalDevice, m_DepthImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_DepthImage, nullptr);
    vkFreeMemory(m_LogicalDevice, m_DepthImageMemory, nullptr);

    vkDestroyImageView(m_LogicalDevice, m_ColorImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_ColorImage, nullptr);
    vkFreeMemory(m_LogicalDevice, m_ColorImageMemory, nullptr);

    vkDestroySampler(m_LogicalDevice, m_ViewportSampler, nullptr);
    vkDestroyImageView(m_LogicalDevice, m_ViewportImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_ViewportImage, nullptr);
    vkFreeMemory(m_LogicalDevice, m_ViewportImageMemory, nullptr);

    vkDestroyImageView(m_LogicalDevice, m_PickingImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_PickingImage, nullptr);
    vkFreeMemory(m_LogicalDevice, m_PickingMemory, nullptr);
    vkDestroyImageView(m_LogicalDevice, m_PickingResolveView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_PickingResolve, nullptr);
    vkFreeMemory(m_LogicalDevice, m_PickingResolveMemory, nullptr);

    for (auto& frame_buffer: m_SwapChainFramebuffers)
    {
        frame_buffer.destroyInnerState();
    }
}

void omp::Renderer::onViewportResize(size_t /*imageIndex*/)
{
    if (m_RenderViewport->isResized() || m_FramebufferResized)
    {
        uint32_t x = static_cast<uint32_t>(m_RenderViewport->getSize().x);
        uint32_t y = static_cast<uint32_t>(m_RenderViewport->getSize().y);
        if (x <= 0 || x >= m_DeviceLimits.maxFramebufferWidth)
        {
            return;
        }
        if (y <= 0 || y >= m_DeviceLimits.maxFramebufferHeight)
        {
            return;
        }

        destroyMainRenderPassResources();

        createColorResources();
        createDepthResources();
        createViewportResources();
        createPickingResources();

        for (size_t index = 0; index < m_SwapChainFramebuffers.size(); index++)
        {
            createFramebufferAtImage(index);
        }
        if (m_ViewportDescriptor != VK_NULL_HANDLE)
        {
            ImGui_ImplVulkan_RemoveTexture(m_ViewportDescriptor);
        }
        m_ViewportDescriptor =
                ImGui_ImplVulkan_AddTexture(m_ViewportSampler, m_ViewportImageView,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_RenderViewport->setImageId(reinterpret_cast<ImTextureID>(m_ViewportDescriptor));
    }
}

omp::GraphicsPipeline* omp::Renderer::findGraphicsPipeline(const std::string& name)
{
    if (m_Pipelines.find(name) != m_Pipelines.end())
    {
        return m_Pipelines.at(name).get();
    }

    throw "No shader with such name";
}

void omp::Renderer::beginRenderPass(
        omp::RenderPass* inRenderPass,
        VkCommandBuffer inCommandBuffer,
        omp::FrameBuffer& inFrameBuffer,
        const std::vector<VkClearValue>& clearValues,
        VkRect2D rect)
{
    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = inRenderPass->getRenderPass();
    render_pass_begin_info.framebuffer = inFrameBuffer.getVulkanFrameBuffer();
    render_pass_begin_info.renderArea = rect;

    render_pass_begin_info.clearValueCount =
            static_cast<uint32_t>(clearValues.size());
    render_pass_begin_info.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(inCommandBuffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void omp::Renderer::endRenderPass(
        omp::RenderPass* /*inRenderPass*/,
        VkCommandBuffer inCommandBuffer)
{
    vkCmdEndRenderPass(inCommandBuffer);
    if (vkEndCommandBuffer(inCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer");
    }
}

void omp::Renderer::prepareCommandBuffer(
        CommandBufferScope& bufferScope,
        VkCommandPool inCommandPool)
{
    VkCommandBufferAllocateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buffer_info.commandPool = inCommandPool;
    buffer_info.commandBufferCount = 1;
    bufferScope.reAllocate(m_LogicalDevice, inCommandPool, buffer_info);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(bufferScope.buffer, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void omp::Renderer::setViewport(VkCommandBuffer inCommandBuffer)
{
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.height = m_RenderViewport->getSize().y;
    viewport.width = m_RenderViewport->getSize().x;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(inCommandBuffer, 0, 1, &viewport);
}

void omp::Renderer::destroyAllCommandBuffers()
{
    for (auto& scope: m_CommandBuffers)
    {
        scope.clearBuffer(m_LogicalDevice, m_CommandPool);
    }
    for (auto& scope: m_ImguiCommandBuffers)
    {
        scope.clearBuffer(m_LogicalDevice, m_ImguiCommandPool);
    }
}

void omp::Renderer::initializeScene()
{
    // TODO: should go to asset initialization
     auto lambda = [this](ImVec2 pos)
    { m_MousePickingData.push(pos); };
    m_RenderViewport->setMouseClickCallback(lambda);

    m_RenderViewport->setTranslationChangeCallback([this](float inVec[3])
    {
        if (m_CurrentScene->getCurrentEntity())
        {
            m_CurrentScene->getCurrentEntity()->getModelInstance()->getPosition() = glm::vec3(inVec[0], inVec[1], inVec[2]);
        }
    });
    m_RenderViewport->setRotationChangeCallback([this](float inVec[3])
    {
        if (m_CurrentScene->getCurrentEntity())
        {
            glm::vec3 new_rotation{inVec[0], inVec[1], inVec[2]};
            glm::vec3 rotation = m_CurrentScene->getCurrentEntity()->getModelInstance()->getRotation();
            m_CurrentScene->getCurrentEntity()->getModelInstance()->getRotation() += new_rotation - rotation;
        }
    });
    m_RenderViewport->setScaleChangeCallback([this](float inVec[3])
    {
         if (m_CurrentScene->getCurrentEntity())
         {
             m_CurrentScene->getCurrentEntity()->getModelInstance()->getScale() =
                     glm::vec3(inVec[0], inVec[1], inVec[2]);
         }
    });
}

void omp::Renderer::postFrame()
{
    while (!m_MousePickingData.empty())
    {
        ImVec2 mouse_data = m_MousePickingData.back();

        // read mouse coordinate pixel from image
        VkImageSubresourceLayers subres{};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 0;
        subres.baseArrayLayer = 0;
        subres.layerCount = 1;
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 1;
        region.bufferImageHeight = 1;
        region.imageSubresource = subres;
        VkOffset3D offset{};
        offset.x = static_cast<int32_t>(m_RenderViewport->getLocalCursorPos().x);
        offset.y = static_cast<int32_t>(m_RenderViewport->getLocalCursorPos().y);
        offset.z = 0;
        region.imageOffset = offset;
        VkExtent3D extent{};
        extent.height = 1;
        extent.width = 1;
        extent.depth = 1;
        region.imageExtent = extent;

        m_VulkanContext->transitionImageLayout(
                m_PickingResolve, VK_FORMAT_R32_SINT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1);

        VkCommandBuffer buffer = beginSingleTimeCommands();
        vkCmdCopyImageToBuffer(buffer, m_PickingResolve,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               m_PixelReadBuffer, 1, &region);
        endSingleTimeCommands(buffer);

        uint32_t pixel_value = 1;
        void* data;
        vkMapMemory(m_VulkanContext->logical_device, m_PixelReadMemory, 0,
                    sizeof(int32_t), 0, &data);
        memcpy(&pixel_value, data, sizeof(int32_t));
        vkUnmapMemory(m_VulkanContext->logical_device, m_PixelReadMemory);

        m_CurrentScene->setCurrentId(pixel_value);
        INFO(LogRendering, "value {}", pixel_value);

        m_MousePickingData.pop();
    }
}

void omp::Renderer::tick(float deltaTime)
{
    if (!m_CurrentScene) return;
    glm::mat4 projection = glm::perspective(
            glm::radians(m_CurrentScene->getCurrentCamera()->getViewAngle()),
            (float) m_RenderViewport->getSize().x /
            (float) m_RenderViewport->getSize().y,
            m_CurrentScene->getCurrentCamera()->getNearClipping(),
            m_CurrentScene->getCurrentCamera()->getFarClipping());
    // projection[1][1] *= -1;
    uint32_t id = m_CurrentScene->getCurrentId();
    auto ent = m_CurrentScene->getEntity(id);
    glm::mat4 model{};
    if (ent)
    {
        model = ent->getModelInstance()->getTransform();
    }
    m_RenderViewport->sendPickingData({id, projection, model});

    // m_CurrentScene->getEntity("1-1")->getRotation().x += 1*deltaTime;
    m_CurrentScene->getEntity("2-1")->getModelInstance()->getRotation().x +=
            1 * deltaTime;
    // m_CurrentScene->getEntity("dfasdf")->getRotation().x += 1*deltaTime;
    m_CurrentScene->getCurrentCamera()->applyInputs(deltaTime);
}

