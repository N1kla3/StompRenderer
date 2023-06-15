#include "Renderer.h"
#include <vector>
#include <optional>
#include <set>
#include <algorithm>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "stb_image.h"
#include "backends/imgui_impl_glfw.h"
#include "UI/MainLayer.h"
#include "UI/EntityPanel.h"
#include "UI/ScenePanel.h"
#include "UI/CameraPanel.h"
#include "UI/GlobalLightPanel.h"
#include "UI/ViewPort.h"

#include <tiny_obj_loader.h>

#include "Logs.h"

#ifdef NDEBUG
const bool g_EnableValidationLayers = false;
#else
const bool g_EnableValidationLayers = true;
#endif

namespace
{
    const std::vector<const char*> g_ValidationLayers{"VK_LAYER_KHRONOS_validation"};

    const std::vector<const char*> g_DeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

Renderer::Renderer()
        : m_CurrentScene(std::make_shared<omp::Scene>())
{
}

void Renderer::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    postSwapChainInitialize();
    createImageViews();
    createImguiWidgets();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createMaterialManager();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createTextureImage();
    initializeImgui();

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    m_CommandBuffers.resize(m_PresentKHRImagesNum);
    m_ImguiCommandBuffers.resize(m_PresentKHRImagesNum);
    createSyncObjects();

}

void Renderer::mainLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        static auto prev_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - prev_time).count();
        prev_time = current_time;

        glfwPollEvents();
        drawFrame();
        tick(time);
    }

    vkDeviceWaitIdle(m_LogicalDevice);
}

void Renderer::cleanup()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(m_LogicalDevice, m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_LogicalDevice, m_InFlightFences[i], nullptr);
    }

    cleanupSwapChain();

    destroyAllCommandBuffers();
    vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
    vkDestroyCommandPool(m_LogicalDevice, m_ImguiCommandPool, nullptr);

    if (g_EnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }

    for (VkBuffer buffer: m_IndexBuffers)
    {
        vkDestroyBuffer(m_LogicalDevice, buffer, nullptr);
    }
    for (VkDeviceMemory memory: m_IndexBufferMemories)
    {
        vkFreeMemory(m_LogicalDevice, memory, nullptr);
    }

    for (VkBuffer buffer: m_VertexBuffers)
    {
        vkDestroyBuffer(m_LogicalDevice, buffer, nullptr);
    }
    for (VkDeviceMemory memory: m_VertexBufferMemories)
    {
        vkFreeMemory(m_LogicalDevice, memory, nullptr);
    }

    m_UboBuffer.reset();
    m_LightSystem.reset();

    vkDestroyDescriptorSetLayout(m_LogicalDevice, m_UboDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_LogicalDevice, m_TexturesDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, nullptr);

    omp::MaterialManager::getMaterialManager().clearGpuState();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(m_LogicalDevice, m_ImguiDescriptorPool, nullptr);

    vkDestroyDevice(m_LogicalDevice, nullptr);

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

    vkDestroyInstance(m_Instance, nullptr);

    glfwDestroyWindow(m_Window);

    glfwTerminate();
}

void Renderer::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
}

void Renderer::createInstance()
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
        create_info.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
        create_info.ppEnabledLayerNames = g_ValidationLayers.data();
        populateDebugMessengerCreateInfo(debug_create_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }
    auto required_extensions = getRequiredExtensions();
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();


    VkResult result = vkCreateInstance(&create_info, nullptr, &m_Instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to craete vk instance");
    }

    uint32_t ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

    std::vector<VkExtensionProperties> extensions(ext_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, extensions.data());

    INFO(Rendering, "Available extensions");
    for (const auto& ext: extensions)
    {
        INFO(Rendering, ext.extensionName);
    }

    if (g_EnableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available");
    }
}

bool Renderer::checkValidationLayerSupport()
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

std::vector<const char*> Renderer::getRequiredExtensions()
{
    uint32_t glfw_extensions_count = 0;
    const char** glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extensions_count);
    if (g_EnableValidationLayers)
    {
        extensions.push_back("VK_EXT_debug_utils");
    }
    return extensions;
}

void Renderer::setupDebugMessenger()
{
    if (!g_EnableValidationLayers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    populateDebugMessengerCreateInfo(create_info);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &create_info, nullptr, &m_DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to setup debug messenger");
    }
}

void Renderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void Renderer::pickPhysicalDevice()
{
    VkPhysicalDevice phys_device = VK_NULL_HANDLE;
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
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
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
        swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
           && features.geometryShader && indices.IsComplete() && extensions_supported && swap_chain_adequate &&
           supported_features.samplerAnisotropy;
}

Renderer::QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices{};
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i = 0;
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
        i++;
    }

    return indices;
}

void Renderer::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_PhysDevice);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};

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
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = static_cast<uint32_t>(g_DeviceExtensions.size());
    create_info.ppEnabledExtensionNames = g_DeviceExtensions.data();

    if (g_EnableValidationLayers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
        create_info.ppEnabledLayerNames = g_ValidationLayers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }


    if (vkCreateDevice(m_PhysDevice, &create_info, nullptr, &m_LogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(m_LogicalDevice, indices.graphics_family.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_LogicalDevice, indices.present_family.value(), 0, &m_PresentQueue);

    m_VulkanContext = std::make_shared<omp::VulkanContext>(m_LogicalDevice, m_PhysDevice, m_CommandPool,
                                                           m_GraphicsQueue);
    omp::MaterialManager::getMaterialManager().specifyVulkanContext(m_VulkanContext);
    m_RenderPass = std::make_shared<omp::RenderPass>(m_LogicalDevice);
    m_ImguiRenderPass = std::make_shared<omp::RenderPass>(m_LogicalDevice);
}

void Renderer::createSurface()
{
    if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(g_DeviceExtensions.begin(), g_DeviceExtensions.end());

    for (const auto& extension: available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
}

Renderer::SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &format_count, nullptr);

    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &present_mode_count, nullptr);

    if (present_mode_count != 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& available_format: availableFormats)
    {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB
            && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
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

        actual_extent.height =
                std::max<uint32_t>(capabilities.minImageExtent.height,
                                   std::min<uint32_t>(capabilities.maxImageExtent.height,
                                                      actual_extent.height));

        int h, w;
        glfwGetFramebufferSize(m_Window, &w, &h);
        VkExtent2D actual_extent_sec = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
        return actual_extent_sec;
    }
}

void Renderer::createSwapChain()
{
    SwapChainSupportDetails swap_chain_support = querySwapChainSupport(m_PhysDevice);

    VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = chooseSwapPresentMode(swap_chain_support.present_modes);
    VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

    m_PresentKHRImagesNum = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0
        && m_PresentKHRImagesNum > swap_chain_support.capabilities.maxImageCount)
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
    uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

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

    if (vkCreateSwapchainKHR(m_LogicalDevice, &create_info, nullptr, &m_SwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &m_PresentKHRImagesNum, nullptr);
    m_SwapChainImages.resize(m_PresentKHRImagesNum);
    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &m_PresentKHRImagesNum, m_SwapChainImages.data());

    m_SwapChainImageFormat = surface_format.format;
    m_SwapChainExtent = extent;
}

void Renderer::postSwapChainInitialize()
{
    m_LightSystem = std::make_unique<omp::LightSystem>(m_VulkanContext, m_PresentKHRImagesNum);
}

void Renderer::createImageViews()
{
    m_SwapChainImageViews.resize(m_PresentKHRImagesNum);
    for (size_t i = 0; i < m_PresentKHRImagesNum; i++)
    {
        m_SwapChainImageViews[i] = createImageView(m_SwapChainImages[i], m_SwapChainImageFormat,
                                                   VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Renderer::createGraphicsPipeline()
{
    // Light pipeline
    std::shared_ptr<omp::Shader> light_shader = std::make_shared<omp::Shader>(m_VulkanContext, "../SPRV/vertLight.spv",
                                                                              "../SPRV/fragLight.spv");

    std::unique_ptr<omp::GraphicsPipeline> light_pipe = std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    light_pipe->startDefaultCreation();
    light_pipe->createMultisamplingInfo(m_MSAASamples);
    light_pipe->createViewport(m_SwapChainExtent);
    light_pipe->definePushConstant<omp::ModelPushConstant>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    light_pipe->addPipelineSetLayout(m_UboDescriptorSetLayout);
    light_pipe->addPipelineSetLayout(m_TexturesDescriptorSetLayout);
    light_pipe->createShaders(light_shader);
    light_pipe->confirmCreation(m_RenderPass);

    // Simple pipeline
    std::shared_ptr<omp::Shader> shader = std::make_shared<omp::Shader>(m_VulkanContext, "../SPRV/vert.spv",
                                                                        "../SPRV/frag.spv");

    std::unique_ptr<omp::GraphicsPipeline> pipe = std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    pipe->startDefaultCreation();
    pipe->createMultisamplingInfo(m_MSAASamples);
    pipe->createViewport(m_SwapChainExtent);
    pipe->definePushConstant<omp::ModelPushConstant>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    pipe->addPipelineSetLayout(m_UboDescriptorSetLayout);
    pipe->addPipelineSetLayout(m_TexturesDescriptorSetLayout);
    pipe->createShaders(shader);
    pipe->confirmCreation(m_RenderPass);

    m_Pipelines.insert({"Light", std::move(light_pipe)});
    m_Pipelines.insert({"Simple", std::move(pipe)});
}

void Renderer::createRenderPass()
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

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = findDepthFormat();
    depth_attachment.samples = m_MSAASamples;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attach_ref{};
    depth_attach_ref.attachment = 1;
    depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = m_SwapChainImageFormat;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attach_ref;
    subpass.pResolveAttachments = &color_attachment_resolve_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_RenderPass->startConfiguration();

    m_RenderPass->addAttachment(std::move(color_attachment));
    m_RenderPass->addAttachment(std::move(depth_attachment));
    m_RenderPass->addAttachment(std::move(color_attachment_resolve));

    // reference leak if render pass not created in this method
    m_RenderPass->addSubpass(std::move(subpass));
    m_RenderPass->addDependency(std::move(dependency));

    m_RenderPass->endConfiguration();
}

void Renderer::createFramebuffers()
{
    m_SwapChainFramebuffers.resize(m_PresentKHRImagesNum);

    for (size_t i = 0; i < m_PresentKHRImagesNum; i++)
    {
        createFramebufferAtImage(i);
    }
}

void Renderer::createFramebufferAtImage(size_t index)
{
    std::vector<VkImageView> attachments{m_ColorImageView, m_DepthImageView, m_SwapChainImageViews[index]};
    omp::FrameBuffer frame_buffer(m_LogicalDevice, attachments, m_RenderPass, m_SwapChainExtent.width, m_SwapChainExtent.height);
    m_SwapChainFramebuffers[index] = frame_buffer;
}

void Renderer::createCommandPool()
{
    QueueFamilyIndices queue_family_indices = findQueueFamilies(m_PhysDevice);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr, &m_CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool");
    }
}

void Renderer::prepareFrameForImage(size_t KHRImageIndex)
{
    prepareCommandBuffer(m_ImguiCommandBuffers[KHRImageIndex], m_ImguiCommandPool);

    std::vector<VkClearValue> clear_value{1};
    clear_value[0].color = g_ClearColor;

    VkRect2D rect{};
    rect.extent.height = m_SwapChainExtent.height;
    rect.extent.width = m_SwapChainExtent.width;
    beginRenderPass(m_ImguiRenderPass.get(), m_ImguiCommandBuffers[KHRImageIndex].buffer, m_ImguiFramebuffers[KHRImageIndex], clear_value, rect);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    renderAllUi();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ImguiCommandBuffers[KHRImageIndex].buffer);

    endRenderPass(m_ImguiRenderPass.get(), m_ImguiCommandBuffers[KHRImageIndex].buffer);


    // Main Render pass
    VkCommandBuffer& main_buffer = m_CommandBuffers[KHRImageIndex].buffer;
    prepareCommandBuffer(m_CommandBuffers[KHRImageIndex], m_CommandPool);

    std::vector<VkClearValue> clear_values{2};
    clear_values[0].color = g_ClearColor;
    clear_values[1].depthStencil = {1.0f, 0};

    //rect.offset.x = m_RenderViewport->getOffset().x;
    //rect.offset.y = m_RenderViewport->getOffset().y;
    //rect.extent.height = m_RenderViewport->getSize().y;
    //rect.extent.width = m_RenderViewport->getSize().x;
    beginRenderPass(m_RenderPass.get(), main_buffer, m_SwapChainFramebuffers[KHRImageIndex], clear_values, rect);
    //setViewport(main_buffer);

    VkDeviceSize offsets[] = {0};
    for (size_t index = 0; index < m_CurrentScene->getModels().size(); index++)
    {
        auto& current_model = m_CurrentScene->getModels()[index];
        auto& material_instance = current_model->getMaterialInstance();
        auto material = material_instance->getStaticMaterial().lock();
        if (!material)
        {
            WARN(Rendering, "Material is invalid in material instance");
        }

        VkPipeline model_pipeline = findGraphicsPipeline(material->getShaderName())->getGraphicsPipeline();
        VkPipelineLayout model_pipeline_layout = findGraphicsPipeline(material->getShaderName())->getPipelineLayout();

        vkCmdBindPipeline(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model_pipeline);


        vkCmdBindDescriptorSets(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                model_pipeline_layout,
                                0, 1, &m_UboDescriptorSets[KHRImageIndex],
                                0, nullptr);

        vkCmdBindVertexBuffers(main_buffer, 0, 1, &m_VertexBuffers[index], offsets);
        vkCmdBindIndexBuffer(main_buffer, m_IndexBuffers[index], 0, VK_INDEX_TYPE_UINT32);

        omp::ModelPushConstant constant{ current_model->getTransform(), material_instance->getAmbient(), material_instance->getDiffusive(), material_instance->getSpecular() };
        vkCmdPushConstants(main_buffer,
                           model_pipeline_layout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(omp::ModelPushConstant), &constant);


        // TODO: MATERIALS ARE TOTAL SHIT
        if (material)
        {
            retrieveMaterialRenderState(material);
            vkCmdBindDescriptorSets(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    model_pipeline_layout,
                                    1, 1, &material->getDescriptorSet()[KHRImageIndex],
                                    0, nullptr);
        }
        else
        {
            // default material
            vkCmdBindDescriptorSets(main_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    model_pipeline_layout,
                                    1, 1, &m_DefaultMaterial->getDescriptorSet()[KHRImageIndex],
                                    0, nullptr);
        }
        vkCmdDrawIndexed(main_buffer,
                         static_cast<uint32_t>(m_CurrentScene->getModels()[index]->getIndices().size()), 1, 0, 0, 0);
    }

    endRenderPass(m_RenderPass.get(), main_buffer);
}

void Renderer::drawFrame()
{
    vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
            m_LogicalDevice,
            m_SwapChain,
            UINT64_MAX,
            m_ImageAvailableSemaphores[m_CurrentFrame],
            VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    else
    {
        prepareFrameForImage(image_index);
    }
    //onViewportResize(image_index);

    if (m_ImagesInFlight[image_index] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_LogicalDevice, 1, &m_ImagesInFlight[image_index], VK_TRUE, UINT64_MAX);
    }
    m_ImagesInFlight[image_index] = m_InFlightFences[m_CurrentFrame];

    updateUniformBuffer(image_index);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    std::array<VkCommandBuffer, 2> command_buffers{m_ImguiCommandBuffers[image_index].buffer, m_CommandBuffers[image_index].buffer};
    submit_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    submit_info.pCommandBuffers = command_buffers.data();

    VkSemaphore signal_semaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
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
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
    {
        m_FramebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }
    vkQueueWaitIdle(m_PresentQueue);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::createSyncObjects()
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
        if ((vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS)
            ||
            (vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
            || (vkCreateFence(m_LogicalDevice, &fence_info, nullptr, &m_InFlightFences[i]) != VK_SUCCESS))
        {
            throw std::runtime_error("failed to create sync objects for a frame");
        }
    }
}

void Renderer::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_Window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_LogicalDevice);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorSets();

    createImguiRenderPass();
    createImguiFramebuffers();

    ImGui_ImplVulkan_SetMinImageCount(2);
}

void Renderer::cleanupSwapChain()
{
    vkDestroyImageView(m_LogicalDevice, m_DepthImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_DepthImage, nullptr);
    vkFreeMemory(m_LogicalDevice, m_DepthImageMemory, nullptr);

    vkDestroyImageView(m_LogicalDevice, m_ColorImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_ColorImage, nullptr);
    vkFreeMemory(m_LogicalDevice, m_ColorImageMemory, nullptr);

    for (auto& frame_buffer : m_SwapChainFramebuffers)
    {
        frame_buffer.destroyInnerState();
    }
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

    // sus, why reset materials?
    vkFreeDescriptorSets(m_LogicalDevice, m_DescriptorPool, m_MaterialSets.size(), m_MaterialSets.data());
    m_MaterialSets.clear();
    for (auto& [u, material] : omp::MaterialManager::getMaterialManager().getMaterials())
    {
        material->clearDescriptorSets();
    }

    vkFreeDescriptorSets(m_LogicalDevice, m_DescriptorPool, m_UboDescriptorSets.size(), m_UboDescriptorSets.data());

    m_ImguiRenderPass->destroyInnerState();
}

void Renderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    app->m_FramebufferResized = true;
}

void Renderer::createBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
        VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_LogicalDevice, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex buffer");
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_LogicalDevice, &allocate_info, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory");
    }

    vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);
}

void Renderer::loadModelToBuffer(const omp::Model& model)
{
    VkDeviceSize buffer_size = sizeof(model.getVertices()[0]) * model.getVertices().size();

    // Vertex buffer
    size_t index = m_VertexBuffers.size();
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, staging_buffer_memory);

    void* data;
    vkMapMemory(m_LogicalDevice, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model.getVertices().data(), (size_t) buffer_size);
    vkUnmapMemory(m_LogicalDevice, staging_buffer_memory);

    createVertexBufferAndMemoryAtIndex(index);

    createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffers[index], m_VertexBufferMemories[index]);

    copyBuffer(staging_buffer, m_VertexBuffers[index], buffer_size);

    vkDestroyBuffer(m_LogicalDevice, staging_buffer, nullptr);
    vkFreeMemory(m_LogicalDevice, staging_buffer_memory, nullptr);

    // Index buffer
    index = m_IndexBuffers.size();
    buffer_size = sizeof(model.getIndices()[0]) * model.getIndices().size();

    createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, staging_buffer_memory);

    vkMapMemory(m_LogicalDevice, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model.getIndices().data(), (size_t) buffer_size);
    vkUnmapMemory(m_LogicalDevice, staging_buffer_memory);

    createIndexBufferAndMemoryAtIndex(index);

    createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBuffers[index], m_IndexBufferMemories[index]);

    copyBuffer(staging_buffer, m_IndexBuffers[index], buffer_size);

    vkDestroyBuffer(m_LogicalDevice, staging_buffer, nullptr);
    vkFreeMemory(m_LogicalDevice, staging_buffer_memory, nullptr);
}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysDevice, &memory_properties);
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i))
            && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type");
}

void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer command_buffer = beginSingleTimeCommands();

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, srcBuffer, dstBuffer, 1, &copy_region);

    endSingleTimeCommands(command_buffer);
}

void Renderer::createDescriptorSetLayout()
{
    // TODO: need abstraction

    // UBO LAYOUT
    {
        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = 0;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ubo_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding global_light_layout_binding{};
        global_light_layout_binding.binding = 1;
        global_light_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        global_light_layout_binding.descriptorCount = 1;
        global_light_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        global_light_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding point_light_layout_binding{};
        point_light_layout_binding.binding = 2;
        point_light_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        point_light_layout_binding.descriptorCount = m_LightSystem->getPointLightSize();
        point_light_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        point_light_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding spot_light_layout_binding{};
        spot_light_layout_binding.binding = 3;
        spot_light_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        spot_light_layout_binding.descriptorCount = m_LightSystem->getSpotLightSize();
        spot_light_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        spot_light_layout_binding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 2> ubo_bindings =
            {
                ubo_layout_binding,
                global_light_layout_binding,
                //point_light_layout_binding,
                //spot_light_layout_binding
            };

        VkDescriptorSetLayoutCreateInfo ubo_layout_info{};
        ubo_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ubo_layout_info.bindingCount = static_cast<uint32_t>(ubo_bindings.size());
        ubo_layout_info.pBindings = ubo_bindings.data();

        if (vkCreateDescriptorSetLayout(m_LogicalDevice, &ubo_layout_info, nullptr, &m_UboDescriptorSetLayout) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ubo descriptor set layout!");
        }
    }

    // TEXTURES LAYOUT
    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 0;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding diffusive_layout_binding{};
    diffusive_layout_binding.binding = 1;
    diffusive_layout_binding.descriptorCount = 1;
    diffusive_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    diffusive_layout_binding.pImmutableSamplers = nullptr;
    diffusive_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding specular_layout_binding{};
    specular_layout_binding.binding = 2;
    specular_layout_binding.descriptorCount = 1;
    specular_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    specular_layout_binding.pImmutableSamplers = nullptr;
    specular_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> texture_bindings = {sampler_layout_binding, diffusive_layout_binding, specular_layout_binding};

    VkDescriptorSetLayoutCreateInfo texture_layout_info{};
    texture_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    texture_layout_info.bindingCount = static_cast<uint32_t>(texture_bindings.size());
    texture_layout_info.pBindings = texture_bindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &texture_layout_info, nullptr, &m_TexturesDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture descriptor set layout!");
    }
}

void Renderer::createUniformBuffers()
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    m_UboBuffer = std::make_unique<omp::UniformBuffer>(m_VulkanContext, m_PresentKHRImagesNum, buffer_size);

    m_LightSystem->recreate();
}

void Renderer::updateUniformBuffer(uint32_t currentImage)
{
    UniformBufferObject ubo{};
    ubo.view = m_CurrentScene->getCurrentCamera()->getViewMatrix();
    ubo.proj = glm::perspective(
            glm::radians(m_CurrentScene->getCurrentCamera()->getViewAngle()),
            (float) m_RenderViewport->getSize().x / (float) m_RenderViewport->getSize().y,
            m_CurrentScene->getCurrentCamera()->getNearClipping(), m_CurrentScene->getCurrentCamera()->getFarClipping());
    ubo.proj[1][1] *= -1;
    ubo.view_position = m_CurrentScene->getCurrentCamera()->getPosition();

    m_UboBuffer->mapMemory(ubo, currentImage);

    m_LightSystem->update();
    m_LightSystem->mapMemory(currentImage);
}

void Renderer::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // TODO think about size
    pool_sizes[0].descriptorCount = 100;

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = 10;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = 1000;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void Renderer::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(m_PresentKHRImagesNum, m_UboDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = m_DescriptorPool;
    allocate_info.descriptorSetCount = m_PresentKHRImagesNum;
    allocate_info.pSetLayouts = layouts.data();

    m_UboDescriptorSets.resize(m_PresentKHRImagesNum);
    if (vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info, m_UboDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_PresentKHRImagesNum; i++)
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

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
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

        #if 0
        descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet = m_UboDescriptorSets[i];
        descriptor_writes[2].dstBinding = 2;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pBufferInfo = &point_light_info;

        descriptor_writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[3].dstSet = m_UboDescriptorSets[i];
        descriptor_writes[3].dstBinding = 3;
        descriptor_writes[3].dstArrayElement = 0;
        descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[3].descriptorCount = 1;
        descriptor_writes[3].pBufferInfo = &spot_light_info;
        #endif

        vkUpdateDescriptorSets(m_LogicalDevice,
                               static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
    }
}

void Renderer::retrieveMaterialRenderState(const std::shared_ptr<omp::Material>& material)
{
    if (!material)
    {
        VWARN(Rendering, "Material is invalid");
        return;
    }
    if (material->isPotentiallyReadyForRendering())
    {
        return;
    }

    std::vector<VkDescriptorSet> ds;
    std::vector<VkDescriptorSetLayout> layouts(m_PresentKHRImagesNum, m_TexturesDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = m_DescriptorPool;
    allocate_info.descriptorSetCount = m_PresentKHRImagesNum;
    allocate_info.pSetLayouts = layouts.data();

    ds.resize(m_PresentKHRImagesNum);
    auto err = vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info, ds.data());
    if (err != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    const omp::MaterialRenderInfo* const material_render_info = material->getRenderInfo();

    // TODO performance
    for (size_t index = 0; index < ds.size(); index++)
    {
        std::vector<VkWriteDescriptorSet> descriptor_writes{};
        descriptor_writes.reserve(ds.size());

        std::vector<VkDescriptorImageInfo> image_infos;
        image_infos.reserve(material_render_info->textures.size());

        for (auto& data : material_render_info->textures)
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
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pImageInfo = &image_infos.at(data.binding_index);

            descriptor_writes.push_back(descriptor_write);
        }

        vkUpdateDescriptorSets(m_LogicalDevice,
                               static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
    }
    m_MaterialSets.insert(m_MaterialSets.begin(), ds.begin(), ds.end());

    material->setDescriptorSet(ds);
}

void Renderer::createTextureImage()
{
    omp::MaterialManager::getMaterialManager().loadTextureLazily("../textures/viking.png");
    omp::MaterialManager::getMaterialManager().loadTextureLazily("../textures/container.png");
    omp::MaterialManager::getMaterialManager().loadTextureLazily("../textures/container_specular.png");

    m_DefaultMaterial = omp::MaterialManager::getMaterialManager().createOrGetMaterial("default");
    // TODO: remove hardcoding
    m_DefaultMaterial->addTexture(omp::ETextureType::Texture,
                                  omp::MaterialManager::getMaterialManager().getTexture("../textures/container.png"));
    m_DefaultMaterial->addTexture(omp::ETextureType::DiffusiveMap,
                                  omp::MaterialManager::getMaterialManager().getTexture("../textures/container_specular.png"));
    m_DefaultMaterial->addTexture(omp::ETextureType::SpecularMap,
                                  omp::MaterialManager::getMaterialManager().getTexture("../textures/container_specular.png"));
    m_DefaultMaterial->setShaderName("Light");
}

void Renderer::createImage(
        uint32_t width, uint32_t height, uint32_t mipLevels,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory,
        VkSampleCountFlagBits numSamples)
{
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(width);
    image_info.extent.height = static_cast<uint32_t>(height);
    image_info.extent.depth = 1;
    image_info.mipLevels = mipLevels;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = numSamples;
    image_info.flags = 0;

    if (vkCreateImage(m_LogicalDevice, &image_info, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image!");
    }

    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(m_LogicalDevice, image, &mem_req);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = mem_req.size;
    allocate_info.memoryTypeIndex = findMemoryType(mem_req.memoryTypeBits, properties);

    if (vkAllocateMemory(m_LogicalDevice, &allocate_info, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    vkBindImageMemory(m_LogicalDevice, image, imageMemory, 0);
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
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

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
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

void Renderer::transitionImageLayout(
        VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer command_buffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(command_buffer,
                         source_stage, destination_stage,
                         0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    endSingleTimeCommands(command_buffer);
}

VkImageView
Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspectFlags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = mipLevels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (vkCreateImageView(m_LogicalDevice, &view_info, nullptr, &image_view) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture image view!");
    }

    return image_view;
}

void Renderer::createDepthResources()
{
    VkFormat depth_format = findDepthFormat();
    createImage(m_SwapChainExtent.width, m_SwapChainExtent.height, 1, depth_format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_DepthImage, m_DepthImageMemory, m_MSAASamples);
    m_DepthImageView = createImageView(m_DepthImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    transitionImageLayout(m_DepthImage, depth_format, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

}

VkFormat Renderer::findSupportedFormat(
        const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format: candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_PhysDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format!");
}

VkFormat Renderer::findDepthFormat()
{
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Renderer::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

std::shared_ptr<omp::Model> Renderer::loadModel(const std::string& name, const std::string& modelName)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelName.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    m_UniqueVertices.clear();

    omp::Model loaded_model;

    for (const auto& shape: shapes)
    {
        for (const auto& index: shape.mesh.indices)
        {
            omp::Vertex vertex{};

            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1 - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
            };
            // TODO incorrect amount

            if (m_UniqueVertices.count(vertex) == 0)
            {
                m_UniqueVertices[vertex] = static_cast<uint32_t>(loaded_model.getVertices().size());
                loaded_model.addVertex(vertex);
            }

            loaded_model.addIndex(m_UniqueVertices[vertex]);
        }
    }

    loaded_model.setName(name);
    // TODO should be choosable
    loaded_model.setMaterial(m_DefaultMaterial);
    loadModelToBuffer(loaded_model);
    auto model_ptr = std::make_shared<omp::Model>(loaded_model);
    m_CurrentScene->addModelToScene(model_ptr);
    return model_ptr;
}

void Renderer::createImguiContext()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.Fonts->Build();
    ImGui::StyleColorsDark();
}

void Renderer::initializeImgui()
{
    createImguiContext();
    createImguiRenderPass();

    VkDescriptorPoolSize pool_sizes[] =
            {
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
                    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
            };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr, &m_ImguiDescriptorPool);

    ImGui_ImplGlfw_InitForVulkan(m_Window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_Instance;
    init_info.PhysicalDevice = m_PhysDevice;
    init_info.Device = m_LogicalDevice;
    init_info.QueueFamily = findQueueFamilies(m_PhysDevice).graphics_family.value();
    init_info.Queue = m_GraphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_ImguiDescriptorPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = m_PresentKHRImagesNum;

    // Imgui render pass should be created before call of this method
    ImGui_ImplVulkan_Init(&init_info, m_ImguiRenderPass->getRenderPass());

    VkCommandBuffer command_buffer = beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    endSingleTimeCommands(command_buffer);

    createImguiFramebuffers();
    createImguiCommandPools();
}

void Renderer::createImguiRenderPass()
{
    VkAttachmentDescription attachment{};
    attachment.format = m_SwapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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

void Renderer::createImguiCommandPools()
{
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = findQueueFamilies(m_PhysDevice).graphics_family.value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr, &m_ImguiCommandPool))
    {
        throw std::runtime_error("Failed to create imgui command pool");
    }
}

void Renderer::createImguiFramebuffers()
{
    m_ImguiFramebuffers.resize(m_PresentKHRImagesNum);
    for (size_t i = 0; i < m_PresentKHRImagesNum; i++)
    {
        omp::FrameBuffer frame_buffer(m_LogicalDevice, {m_SwapChainImageViews[i]}, m_ImguiRenderPass, m_SwapChainExtent.width, m_SwapChainExtent.height);
        m_ImguiFramebuffers[i] = frame_buffer;
    }
}

VkSampleCountFlagBits Renderer::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(m_PhysDevice, &physical_device_properties);

    VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts
                                & physical_device_properties.limits.framebufferDepthSampleCounts;

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

void Renderer::createColorResources()
{
    VkFormat color_format = m_SwapChainImageFormat;

    createImage(m_SwapChainExtent.width, m_SwapChainExtent.height, 1,
                color_format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ColorImage, m_ColorImageMemory,
                m_MSAASamples);
    m_ColorImageView = createImageView(m_ColorImage, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

}

void Renderer::createVertexBufferAndMemoryAtIndex(size_t index)
{
    if (m_VertexBuffers.size() >= index)
    {
        while (m_VertexBuffers.size() != index + 1)
        {
            m_VertexBuffers.push_back({});
        }
    }

    if (m_VertexBufferMemories.size() >= index)
    {
        while (m_VertexBufferMemories.size() != index + 1)
        {
            m_VertexBufferMemories.push_back({});
        }
    }
}

void Renderer::createIndexBufferAndMemoryAtIndex(size_t index)
{
    if (m_IndexBuffers.size() >= index)
    {
        while (m_IndexBuffers.size() != index + 1)
        {
            m_IndexBuffers.push_back({});
        }
    }

    if (m_IndexBufferMemories.size() >= index)
    {
        while (m_IndexBufferMemories.size() != index + 1)
        {
            m_IndexBufferMemories.push_back({});
        }
    }
}

void Renderer::renderAllUi()
{
    static auto prev_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - prev_time).count();
    prev_time = current_time;

    for (auto& unit: m_Widgets)
    {
        unit->renderUi(time);
    }

    // TODO remove
    ImGui::Begin("Imagessss");
    if (omp::MaterialManager::getMaterialManager().getTexture("../textures/viking.png"))
    {
        ImGui::Image(
                (ImTextureID) (omp::MaterialManager::getMaterialManager().getTexture("../textures/viking.png")->getTextureId()), {100, 100});
    }

    ImGui::End();

    ImGui::ShowDemoWindow();
}

void Renderer::createImguiWidgets()
{
    // ORDER IS IMPORTANT DOCK NODES GO FIRST

    m_RenderViewport = std::make_shared<omp::ViewPort>();
    m_RenderViewport->setCamera(m_CurrentScene->getCurrentCamera());
    auto material_panel = std::make_shared<omp::MaterialPanel>();
    auto entity = std::make_shared<omp::EntityPanel>(material_panel);
    m_ScenePanel = std::make_shared<omp::ScenePanel>(entity);
    m_ScenePanel->setScene(m_CurrentScene);
    auto camera_panel = std::make_shared<omp::CameraPanel>(m_CurrentScene->getCurrentCamera());
    // auto light_panel = std::make_shared<omp::GlobalLightPanel>(m_GlobalLight);

    m_Widgets.push_back(std::make_shared<omp::MainLayer>());
    m_Widgets.push_back(m_RenderViewport);
    m_Widgets.push_back(std::move(entity));
    m_Widgets.push_back(std::move(material_panel));
    m_Widgets.push_back(m_ScenePanel);
    m_Widgets.push_back(std::move(camera_panel));
    // TODO: light ui
    //m_Widgets.push_back(std::move(light_panel));
}

void Renderer::onViewportResize(size_t imageIndex)
{
    m_SwapChainFramebuffers[imageIndex].destroyInnerState();
    createFramebufferAtImage(imageIndex);
    prepareFrameForImage(imageIndex);
}

void Renderer::createMaterialManager()
{
    m_VulkanContext->setCommandPool(m_CommandPool);
}

omp::GraphicsPipeline* Renderer::findGraphicsPipeline(const std::string& name)
{
    if (m_Pipelines.find(name) != m_Pipelines.end())
    {
        return m_Pipelines.at(name).get();
    }

    throw "No shader with such name";
}

void Renderer::beginRenderPass(
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

    render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_begin_info.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(inCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::endRenderPass(omp::RenderPass* inRenderPass, VkCommandBuffer inCommandBuffer)
{
    vkCmdEndRenderPass(inCommandBuffer);
    if (vkEndCommandBuffer(inCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer");
    }
}

void Renderer::prepareCommandBuffer(CommandBufferScope& bufferScope, VkCommandPool inCommandPool)
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

void Renderer::setViewport(VkCommandBuffer inCommandBuffer)
{
    VkViewport viewport;
    viewport.x = m_RenderViewport->getOffset().x;
    viewport.y = m_RenderViewport->getOffset().y;
    viewport.height = m_RenderViewport->getSize().y;
    viewport.width = m_RenderViewport->getSize().x;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(inCommandBuffer, 0, 1, &viewport);
}

void Renderer::destroyAllCommandBuffers()
{
    for (auto& scope : m_CommandBuffers)
    {
        scope.clearBuffer(m_LogicalDevice, m_CommandPool);
    }
    for (auto& scope : m_ImguiCommandBuffers)
    {
        scope.clearBuffer(m_LogicalDevice, m_ImguiCommandPool);
    }
}

void Renderer::initializeScene()
{
    // TODO: model split
    loadModel("First", g_ModelPath.c_str())->getPosition() = {10.f, 3.f, 4.f};
    loadModel("Second", g_ModelPath.c_str())->getPosition() = {20.f, 3.f, 4.f};
    loadModel("third", g_ModelPath.c_str())->getPosition() = {30.f, 3.f, 4.f};
    loadModel("fourth", g_ModelPath.c_str())->getPosition() = {40.f, 3.f, 4.f};

    auto model = loadModel("I see the light", "../models/sphere.obj");
    auto mat = omp::MaterialManager::getMaterialManager().createOrGetMaterial("default_no_light");
    mat->addTexture(omp::ETextureType::Texture, omp::MaterialManager::getMaterialManager().getDefaultTexture().lock());
    mat->setShaderName("Simple");
    model->setMaterial(mat);

    m_LightSystem->setModelForEach(model);
}

void Renderer::tick(float deltaTime)
{
    m_CurrentScene->getModel("fourth")->getRotation().x += 1*deltaTime;
    m_CurrentScene->getCurrentCamera()->applyInputs(deltaTime);
}

