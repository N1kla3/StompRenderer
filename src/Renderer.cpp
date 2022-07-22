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
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

namespace
{
    const std::vector<const char*> ValidationLayers{ "VK_LAYER_KHRONOS_validation" };

    const std::vector<const char*> DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

Renderer::Renderer()
    : m_CurrentScene(std::make_shared<omp::Scene>())
    , m_Camera(std::make_shared<omp::Camera>())
    , m_GlobalLight(std::make_shared<omp::Light>())
{
    m_LightObject = std::make_shared<omp::LightObject>();
    m_LightObject->SetLight(m_GlobalLight);
}

void Renderer::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
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
    InitializeImgui();
    loadModel("First", MODEL_PATH.c_str());
    loadModel("Second", MODEL_PATH.c_str());
    loadLightObject("I see the light", "../models/cube.obj");
    //loadModel("Third");
    //loadModel("First1");
    //loadModel("Second1");
    //loadModel("Third1");
    //loadModel("First2");
    //loadModel("Second2");
    //loadModel("Third2");
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();

}

void Renderer::mainLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
        drawFrame();
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

    m_MaterialManager.reset();
    cleanupSwapChain();

    vkDestroyCommandPool(m_LogicalDevice, m_CommandPools, nullptr);

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, debugMessenger, nullptr);
    }

    for (VkBuffer buffer : m_IndexBuffers)
    {
        vkDestroyBuffer(m_LogicalDevice, buffer, nullptr);
    }
    for (VkDeviceMemory memory : m_IndexBufferMemories)
    {
        vkFreeMemory(m_LogicalDevice, memory, nullptr);
    }

    for (VkBuffer buffer : m_VertexBuffers)
    {
        vkDestroyBuffer(m_LogicalDevice, buffer, nullptr);
    }
    for (VkDeviceMemory memory : m_VertexBufferMemories)
    {
        vkFreeMemory(m_LogicalDevice, memory, nullptr);
    }

    vkDestroyDescriptorSetLayout(m_LogicalDevice, m_DescriptorSetLayout, nullptr);

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
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        create_info.ppEnabledLayerNames = ValidationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }
    auto extensions_ = getRequiredExtensions();
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions_.size());
    create_info.ppEnabledExtensionNames = extensions_.data();


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
    for (const auto& ext : extensions)
    {
        INFO(Rendering, ext.extensionName);
    }

    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw  std::runtime_error("validation layers requested, but not available");
    }
}

bool Renderer::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    bool layer_found = false;

    for (const char* layerName : ValidationLayers)
    {
        for (const auto& layerProperty : availableLayers)
        {
            if (strcmp(layerName, layerProperty.layerName) == 0)
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

std::vector<const char *> Renderer::getRequiredExtensions()
{
    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
    if (enableValidationLayers)
    {
        extensions.push_back("VK_EXT_debug_utils");
    }
    return extensions;
}

void Renderer::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    populateDebugMessengerCreateInfo(create_info);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &create_info, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to setup debug messenger");
    }
}

void Renderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
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
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &DeviceCount, nullptr);
    if (DeviceCount == 0)
    {
        throw std::runtime_error("failed to find GPU physical devices");
    }

    std::vector<VkPhysicalDevice> devices(DeviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &DeviceCount, devices.data());

    for (const auto& device : devices)
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
        && features.geometryShader && indices.isComplete() && extensions_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
}

Renderer::QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices{};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queue_families.data());

    int i = 0;
    for (const auto& queue : queue_families)
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
    for (uint32_t queue_family : unique_queue_families)
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

    create_info.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    create_info.ppEnabledExtensionNames = DeviceExtensions.data();

    if (enableValidationLayers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        create_info.ppEnabledLayerNames = ValidationLayers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }


    if (vkCreateDevice(m_PhysDevice, &create_info, nullptr, &m_LogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(m_LogicalDevice, indices.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(m_LogicalDevice, indices.present_family.value(), 0, &present_queue);

    m_VulkanContext = std::make_shared<omp::VulkanContext>(m_LogicalDevice, m_PhysDevice, m_CommandPools, graphics_queue);
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

    std::set<std::string> required_extensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& extension : available_extensions)
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

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto& available_format : availableFormats)
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
    for (const auto& available_presentation_mode : availablePresentModes)
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
        VkExtent2D actualExtent = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
        return actualExtent;
    }
}

void Renderer::createSwapChain()
{
    SwapChainSupportDetails swap_chain_support = querySwapChainSupport(m_PhysDevice);

    VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = chooseSwapPresentMode(swap_chain_support.present_modes);
    VkExtent2D extent = chooseSwapExtent(swap_chain_support.capabilities);

    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0
        && image_count > swap_chain_support.capabilities.maxImageCount)
    {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_Surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(m_PhysDevice);
    uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value() };

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

    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &image_count, nullptr);
    m_SwapChainImages.resize(image_count);
    vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &image_count, m_SwapChainImages.data());

    m_SwapChainImageFormat = surface_format.format;
    m_SwapChainExtent = extent;
}

void Renderer::createImageViews()
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());
    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        m_SwapChainImageViews[i] = createImageView(m_SwapChainImages[i], m_SwapChainImageFormat,
                                                   VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Renderer::createGraphicsPipeline()
{
    m_CurrentShader = std::make_shared<omp::Shader>(m_VulkanContext, "../SPRV/vertLight.spv", "../SPRV/fragLight.spv");

    m_LightGraphicsPipeline = std::make_unique<omp::GraphicsPipeline>(m_LogicalDevice);
    m_LightGraphicsPipeline->StartDefaultCreation();
    m_LightGraphicsPipeline->CreateMultisamplingInfo(m_MSAASamples);
    m_LightGraphicsPipeline->CreateViewport(m_SwapChainExtent);
    m_LightGraphicsPipeline->CreatePipelineLayout(m_DescriptorSetLayout);
    m_LightGraphicsPipeline->CreateShaders(m_CurrentShader);
    m_LightGraphicsPipeline->ConfirmCreation(m_RenderPass);

    m_CurrentShader.reset();
}

void Renderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_SwapChainImageFormat;
    colorAttachment.samples = m_MSAASamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depth_attach_ref;
    subpass.pResolveAttachments = &color_attachment_resolve_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depth_attachment, color_attachment_resolve};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void Renderer::createFramebuffers()
{
    m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
    {
        createFramebufferAtImage(i);
    }
}

void Renderer::createFramebufferAtImage(size_t index)
{
    std::array<VkImageView, 3> attachments = { m_ColorImageView, m_DepthImageView, m_SwapChainImageViews[index] };

    VkFramebufferCreateInfo frame_buffer_info{};
    frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_info.renderPass = m_RenderPass;
    frame_buffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    frame_buffer_info.pAttachments = attachments.data();
    frame_buffer_info.width = m_RenderViewport->GetSize().x;//m_SwapChainExtent.width/2;
    frame_buffer_info.height = m_RenderViewport->GetSize().y;//m_SwapChainExtent.height/2;
    frame_buffer_info.layers = 1;

    if (vkCreateFramebuffer(m_LogicalDevice, &frame_buffer_info, nullptr, &m_SwapChainFramebuffers[index])
        != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void Renderer::createCommandPool()
{
    QueueFamilyIndices queue_family_indices = findQueueFamilies(m_PhysDevice);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr, &m_CommandPools) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool");
    }
}

void Renderer::createCommandBuffers()
{
    m_CommandBuffers.resize(m_SwapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = m_CommandPools;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = (uint32_t) m_CommandBuffers.size();
    allocate_info.pNext = nullptr;

    if (vkAllocateCommandBuffers(m_LogicalDevice, &allocate_info, m_CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers");
    }

    for (size_t i = 0; i < m_CommandBuffers.size(); i++)
    {
        createCommandBufferForImage(i);
    }
}

void Renderer::createCommandBufferForImage(size_t inIndex)
{
    vkFreeCommandBuffers(m_LogicalDevice, m_CommandPools, 1, &m_CommandBuffers[inIndex]);

    VkCommandBufferAllocateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buffer_info.commandPool = m_CommandPools;
    buffer_info.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_LogicalDevice, &buffer_info, &m_CommandBuffers[inIndex]);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_CommandBuffers[inIndex], &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = m_RenderPass;
    render_pass_begin_info.framebuffer = m_SwapChainFramebuffers[inIndex];
    render_pass_begin_info.renderArea.offset.x = m_RenderViewport->GetOffset().x;
    render_pass_begin_info.renderArea.offset.y = m_RenderViewport->GetOffset().y;
    render_pass_begin_info.renderArea.extent.height = m_RenderViewport->GetSize().y;//m_SwapChainExtent.height/2;
    render_pass_begin_info.renderArea.extent.width = m_RenderViewport->GetSize().x;//m_SwapChainExtent.width/2;

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = CLEAR_COLOR;
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_begin_info.pClearValues = clear_values.data();

    VkViewport viewport;
    viewport.x = m_RenderViewport->GetOffset().x;
    viewport.y = m_RenderViewport->GetOffset().y;
    viewport.height = m_RenderViewport->GetSize().y;
    viewport.width = m_RenderViewport->GetSize().x;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdBeginRenderPass(m_CommandBuffers[inIndex], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(m_CommandBuffers[inIndex], 0, 1, &viewport);
    vkCmdBindPipeline(m_CommandBuffers[inIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_LightGraphicsPipeline->GetGraphicsPipeline());

    VkDeviceSize offsets[] = { 0 };

    for (size_t index = 0; index < m_CurrentScene->GetModels().size(); index++)
    {
        auto& current_model = m_CurrentScene->GetModels()[index];
        auto current_mat = current_model->GetMaterial();

        vkCmdBindVertexBuffers(m_CommandBuffers[inIndex], 0, 1, &m_VertexBuffers[index], offsets);
        vkCmdBindIndexBuffer(m_CommandBuffers[inIndex], m_IndexBuffers[index], 0, VK_INDEX_TYPE_UINT32);

        omp::ModelPushConstant constant;
        constant.model = current_model->GetTransform();
        constant.m_Ambient = current_mat->GetAmbient();
        constant.m_Diffusive = current_mat->GetDiffusive();
        constant.m_Specular = current_mat->GetSpecular();
        vkCmdPushConstants(m_CommandBuffers[inIndex], m_LightGraphicsPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(omp::ModelPushConstant), &constant);


        if (current_mat->IsInitialized())
        {
            vkCmdBindDescriptorSets(m_CommandBuffers[inIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_LightGraphicsPipeline->GetPipelineLayout(), 0, 1, &current_mat->GetDescriptorSet()[inIndex], 0, nullptr);
        }
        else
        {
            createDescriptorSetsForMaterial(current_model->GetMaterial());
            vkCmdBindDescriptorSets(m_CommandBuffers[inIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_LightGraphicsPipeline->GetPipelineLayout(), 0, 1, &m_DefaultMaterial->GetDescriptorSet()[inIndex], 0, nullptr);
        }
        vkCmdDrawIndexed(m_CommandBuffers[inIndex], static_cast<uint32_t>(m_CurrentScene->GetModels()[index]->GetIndices().size()), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(m_CommandBuffers[inIndex]);
    if (vkEndCommandBuffer(m_CommandBuffers[inIndex]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer");
    }
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
        //vkFreeCommandBuffers(m_LogicalDevice, m_ImguiCommandPool, 1, &m_ImguiCommandBuffers[image_index]);
        createImguiCommandBufferAtIndex(image_index);
        createCommandBufferForImage(image_index);
    }
    onViewportResize(image_index);

    if (m_ImagesInFlight[image_index] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_LogicalDevice, 1, &m_ImagesInFlight[image_index], VK_TRUE, UINT64_MAX);
    }

    m_ImagesInFlight[image_index] = m_InFlightFences[m_CurrentFrame];

    updateUniformBuffer(image_index);
    //vkResetCommandPool(m_LogicalDevice, m_ImguiCommandPool, 0);


    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    std::array<VkCommandBuffer, 2> command_buffers{ m_ImguiCommandBuffers[image_index], m_CommandBuffers[image_index] };
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = command_buffers.data();

    VkSemaphore signal_semaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

    if (vkQueueSubmit(graphics_queue, 1, &submit_info, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = { m_SwapChain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
    {
        m_FramebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }
    vkQueueWaitIdle(present_queue);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::createSyncObjects()
{
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_ImagesInFlight.resize(m_SwapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if ((vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS)
            || (vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
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
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();

    createImguiRenderPass();
    createImguiFramebuffers();
    createImguiCommandPools();
    createImguiCommandBuffers();

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

    for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(m_LogicalDevice, m_SwapChainFramebuffers[i], nullptr);
    }
    vkFreeCommandBuffers(m_LogicalDevice, m_CommandPools, static_cast<int32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
    m_LightGraphicsPipeline.reset();
    vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, nullptr);

    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
    {
        vkDestroyImageView(m_LogicalDevice, m_SwapChainImageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);

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

    vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, nullptr);

    for (auto framebuffer : m_ImguiFramebuffers) {
        vkDestroyFramebuffer(m_LogicalDevice, framebuffer, nullptr);
    }

    vkDestroyRenderPass(m_LogicalDevice, m_ImguiRenderPass, nullptr);

    vkFreeCommandBuffers(m_LogicalDevice, m_ImguiCommandPool, 1, m_ImguiCommandBuffers.data());
    vkDestroyCommandPool(m_LogicalDevice, m_ImguiCommandPool, nullptr);
}

void Renderer::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    app->m_FramebufferResized = true;
}

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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

void Renderer::loadModelToBuffer(const omp::Model &model)
{
    VkDeviceSize buffer_size = sizeof(model.GetVertices()[0]) * model.GetVertices().size();

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
    memcpy(data, model.GetVertices().data(), (size_t)buffer_size);
    vkUnmapMemory(m_LogicalDevice, staging_buffer_memory);

    CreateVertexBufferAndMemoryAtIndex(index);

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
    buffer_size = sizeof(model.GetIndices()[0]) * model.GetIndices().size();

    createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, staging_buffer_memory);

    vkMapMemory(m_LogicalDevice, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model.GetIndices().data(), (size_t)buffer_size);
    vkUnmapMemory(m_LogicalDevice, staging_buffer_memory);

    CreateIndexBufferAndMemoryAtIndex(index);

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
    //TODO need multiple textures

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightLayoutBinding{};
    lightLayoutBinding.binding = 1;
    lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightLayoutBinding.descriptorCount = 1;
    lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 2;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding, lightLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &layout_info, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to creat descriptor set layout!");
    }
}

void Renderer::createUniformBuffers()
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);

    m_UniformBuffers.resize(m_SwapChainImages.size());
    m_UniformBuffersMemory.resize(m_SwapChainImages.size());

    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        createBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_UniformBuffers[i], m_UniformBuffersMemory[i]);
    }

    buffer_size = sizeof(omp::Light);

    m_LightBuffer.resize(m_SwapChainImages.size());
    m_LightBufferMemory.resize(m_SwapChainImages.size());

    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        createBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_LightBuffer[i], m_LightBufferMemory[i]);
    }
}

void Renderer::updateUniformBuffer(uint32_t currentImage)
{
    UniformBufferObject ubo{};
    ubo.view = m_Camera->GetViewMatrix();
    ubo.proj = glm::perspective(
            glm::radians(m_Camera->GetViewAngle()),
            (float)m_RenderViewport->GetSize().x / (float)m_RenderViewport->GetSize().y,
            m_Camera->GetNearClipping(), m_Camera->GetFarClipping());
    ubo.proj[1][1] *= -1;
    ubo.viewPosition = m_Camera->GetPosition();

    m_LightObject->UpdateLightObject();

    {
        void *data;
        vkMapMemory(m_LogicalDevice, m_UniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(m_LogicalDevice, m_UniformBuffersMemory[currentImage]);
    }

    void* data;
    vkMapMemory(m_LogicalDevice, m_LightBufferMemory[currentImage], 0, sizeof(omp::Light), 0, &data);
    memcpy(data, m_GlobalLight.get(), sizeof(omp::Light));
    vkUnmapMemory(m_LogicalDevice, m_LightBufferMemory[currentImage]);
}

void Renderer::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // TODO think about size
    pool_sizes[0].descriptorCount = 1000;static_cast<uint32_t>(m_SwapChainImages.size());

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = 1000;static_cast<uint32_t>(m_SwapChainImages.size());

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = 1000;static_cast<uint32_t>(m_SwapChainImages.size());

    if (vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void Renderer::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(m_SwapChainImages.size(), m_DescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = m_DescriptorPool;
    allocate_info.descriptorSetCount = static_cast<uint32_t>(m_SwapChainImages.size());
    allocate_info.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(m_SwapChainImages.size());
    if (vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info, m_DescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = m_UniformBuffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo light_info{};
        light_info.buffer = m_LightBuffer[i];
        light_info.offset = 0;
        light_info.range = sizeof(omp::Light);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = m_DefaultTexture->GetImageView();
        image_info.sampler = m_DefaultTexture->GetSampler();

        std::array<VkWriteDescriptorSet, 3> descriptor_writes{};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = m_DescriptorSets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = m_DescriptorSets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = &light_info;

        descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet = m_DescriptorSets[i];
        descriptor_writes[2].dstBinding = 2;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pImageInfo = &image_info;

        vkUpdateDescriptorSets(m_LogicalDevice,
            static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
    }
}

void Renderer::createDescriptorSetsForMaterial(const std::shared_ptr<omp::Material> &material)
{
    // TODO: multiple later
    std::vector<VkDescriptorSet> DS;
    std::vector<VkDescriptorSetLayout> layouts(m_SwapChainImages.size(), m_DescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = m_DescriptorPool;
    allocate_info.descriptorSetCount = static_cast<uint32_t>(m_SwapChainImages.size());
    allocate_info.pSetLayouts = layouts.data();

    DS.resize(m_SwapChainImages.size());
    if (vkAllocateDescriptorSets(m_LogicalDevice, &allocate_info, DS.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    // TODO performance
    for (size_t index = 0; index < m_SwapChainImages.size(); index++)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = m_UniformBuffers[index];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo light_info{};
        light_info.buffer = m_LightBuffer[index];
        light_info.offset = 0;
        light_info.range = sizeof(omp::Light);

        std::vector<VkWriteDescriptorSet> descriptor_writes{};
        auto material_sets = material->GetDescriptorWriteSets();
        descriptor_writes.resize(material_sets.size() + 2);

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = DS[index];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = DS[index];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = &light_info;

        for (size_t index_write = 0; index_write < material_sets.size(); index_write++)
        {
            descriptor_writes[index_write + 2] = material_sets[index_write];
            descriptor_writes[index_write + 2].dstSet = DS[index];
        }

        vkUpdateDescriptorSets(m_LogicalDevice,
                               static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
    }

    material->SetDescriptorSet(DS);
}

void Renderer::createTextureImage()
{
    m_DefaultTexture = m_MaterialManager->LoadTextureLazily(TEXTURE_PATH);
    m_MaterialManager->LoadTextureLazily("../textures/mando.jpg");

    m_DefaultMaterial = std::make_shared<omp::Material>();
    // TODO: remove hardcoding
    m_DefaultMaterial->AddTexture({2, m_DefaultTexture});

    // TODO: Material instancing
}

void Renderer::createImage(
        uint32_t width, uint32_t height, uint32_t mip_levels,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage &image, VkDeviceMemory &imageMemory,
        VkSampleCountFlagBits numSamples)
{
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(width);
    image_info.extent.height = static_cast<uint32_t>(height);
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_levels;
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
    alloc_info.commandPool = m_CommandPools;
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

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);
    vkFreeCommandBuffers(m_LogicalDevice, m_CommandPools, 1, &commandBuffer);
}

void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mip_levels)
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
    barrier.subresourceRange.levelCount = mip_levels;
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
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

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

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels)
{
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspectFlags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = mip_levels;
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

VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
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

void Renderer::loadLightObject(const std::string& Name, const std::string& TextureName)
{
    auto model = loadModel(Name, TextureName);
    // TODO make a lot of light objects
    m_LightObject->SetModel(model);
}

std::shared_ptr<omp::Model> Renderer::loadModel(const std::string &Name, const std::string &ModelName)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ModelName.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    m_UniqueVertices.clear();

    omp::Model loaded_model;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
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
            vertex.color =  {
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
                m_UniqueVertices[vertex] = static_cast<uint32_t>(loaded_model.GetVertices().size());
                loaded_model.AddVertex(vertex);
            }

            loaded_model.AddIndex(m_UniqueVertices[vertex]);
        }
    }

    loaded_model.SetName(Name);
    loaded_model.SetMaterial(m_DefaultMaterial);
    loadModelToBuffer(loaded_model);
    auto model_ptr = std::make_shared<omp::Model>(loaded_model);
    m_CurrentScene->AddModelToScene(model_ptr);
    return model_ptr;
}

void Renderer::createImguiContext()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.Fonts->Build();
    ImGui::StyleColorsDark();
}

void Renderer::InitializeImgui()
{
    createImguiContext();
    createImguiRenderPass();

    VkDescriptorPoolSize pool_sizes[] =
    {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr, &m_ImguiDescriptorPool);

    ImGui_ImplGlfw_InitForVulkan(m_Window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_Instance;
    init_info.PhysicalDevice = m_PhysDevice;
    init_info.Device = m_LogicalDevice;
    init_info.QueueFamily = findQueueFamilies(m_PhysDevice).graphics_family.value();
    init_info.Queue = graphics_queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_ImguiDescriptorPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = m_SwapChainImages.size();

    // Imgui render pass should be created before call of this method
    ImGui_ImplVulkan_Init(&init_info, m_ImguiRenderPass);

    VkCommandBuffer command_buffer = beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    endSingleTimeCommands(command_buffer);

    createImguiFramebuffers();
    createImguiCommandPools();
    createImguiCommandBuffers();
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

    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    if (vkCreateRenderPass(m_LogicalDevice, &info, nullptr, &m_ImguiRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create imgui render pass");
    }
}

void Renderer::createImguiCommandPools()
{
    auto size = m_SwapChainImageViews.size();

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = findQueueFamilies(m_PhysDevice).graphics_family.value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr, &m_ImguiCommandPool))
    {
        throw std::runtime_error("Failed to create imgui command pool");
    }
}

void Renderer::createImguiCommandBuffers()
{
    auto size = m_ImguiFramebuffers.size();
    m_ImguiCommandBuffers.resize(size);
    VkCommandBufferAllocateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buffer_info.commandPool = m_ImguiCommandPool;
    buffer_info.commandBufferCount = size;
    vkAllocateCommandBuffers(m_LogicalDevice, &buffer_info, m_ImguiCommandBuffers.data());

    for (size_t i = 0; i < size; i++)
    {
        VkClearValue clear_value{};
        clear_value.color = CLEAR_COLOR;

        VkRenderPassBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = m_ImguiRenderPass;
        begin_info.framebuffer = m_ImguiFramebuffers[i];
        begin_info.renderArea.extent.width = m_SwapChainExtent.width;
        begin_info.renderArea.extent.height = m_SwapChainExtent.height;
        begin_info.clearValueCount = 1;
        begin_info.pClearValues = &clear_value;

        VkCommandBufferBeginInfo buffer_begin_info{};
        buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        buffer_begin_info.flags = 0;
        buffer_begin_info.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(m_ImguiCommandBuffers[i], &buffer_begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording imgui command buffer!");
        }

        vkCmdBeginRenderPass(m_ImguiCommandBuffers[i], &begin_info, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        renderAllUi();
        ImGui::Render();

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ImguiCommandBuffers[i]);
        vkCmdEndRenderPass(m_ImguiCommandBuffers[i]);
        if (vkEndCommandBuffer(m_ImguiCommandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer");
        }
    }
}

void Renderer::createImguiFramebuffers()
{
    auto size = m_SwapChainImageViews.size();
    m_ImguiFramebuffers.resize(size);
    for (size_t i = 0; i < size; i++)
    {
        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_ImguiRenderPass;
        info.attachmentCount = 1;
        info.pAttachments = &m_SwapChainImageViews[i];
        info.width = m_SwapChainExtent.width;
        info.height = m_SwapChainExtent.height;
        info.layers = 1;

        if (vkCreateFramebuffer(m_LogicalDevice, &info, nullptr, &m_ImguiFramebuffers[i]))
        {
            throw std::runtime_error("Failed to create imgui framebuffer");
        }
    }
}

void Renderer::createImguiCommandBufferAtIndex(uint32_t Index)
{
    vkFreeCommandBuffers(m_LogicalDevice, m_ImguiCommandPool, 1, &m_ImguiCommandBuffers[Index]);

    VkCommandBufferAllocateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buffer_info.commandPool = m_ImguiCommandPool;
    buffer_info.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_LogicalDevice, &buffer_info, &m_ImguiCommandBuffers[Index]);

    VkClearValue clear_value{};
    clear_value.color = CLEAR_COLOR;

    VkRenderPassBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.renderPass = m_ImguiRenderPass;
    begin_info.framebuffer = m_ImguiFramebuffers[Index];
    begin_info.renderArea.extent.width = m_SwapChainExtent.width;
    begin_info.renderArea.extent.height = m_SwapChainExtent.height;
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &clear_value;

    VkCommandBufferBeginInfo buffer_begin_info{};
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buffer_begin_info.flags = 0;
    buffer_begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_ImguiCommandBuffers[Index], &buffer_begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording imgui command buffer!");
    }

    vkCmdBeginRenderPass(m_ImguiCommandBuffers[Index], &begin_info, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    renderAllUi();
    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ImguiCommandBuffers[Index]);
    vkCmdEndRenderPass(m_ImguiCommandBuffers[Index]);
    if (vkEndCommandBuffer(m_ImguiCommandBuffers[Index]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer");
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

void Renderer::CreateVertexBufferAndMemoryAtIndex(size_t index)
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

void Renderer::CreateIndexBufferAndMemoryAtIndex(size_t index)
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
    static auto prevTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
    prevTime = currentTime;

    for (auto& unit : m_Widgets)
    {
        unit->renderUI(time);
    }

    // TODO remove
    ImGui::Begin("Imagessss");
    if (m_MaterialManager->GetTexture("../textures/viking.png"))
        ImGui::Image(
                (ImTextureID)(m_MaterialManager->GetTexture("../textures/viking.png")->GetTextureId()), {100, 100});
    if (m_MaterialManager->GetTexture("../textures/mando.jpg"))
        ImGui::Image(
                (ImTextureID)(m_MaterialManager->GetTexture("../textures/mando.jpg")->GetTextureId()), {100, 100});

    ImGui::End();

    ImGui::ShowDemoWindow();
}

void Renderer::createImguiWidgets()
{
    // ORDER IS IMPORTANT DOCK NODES GO FIRST

    m_RenderViewport = std::make_shared<omp::ViewPort>();
    m_RenderViewport->SetCamera(m_Camera);
    auto material_panel = std::make_shared<omp::MaterialPanel>();
    auto entity = std::make_shared<omp::EntityPanel>(material_panel);
    m_ScenePanel = std::make_shared<omp::ScenePanel>(entity);
    m_ScenePanel->SetScene(m_CurrentScene);
    auto camera_panel = std::make_shared<omp::CameraPanel>(m_Camera);
    auto light_panel = std::make_shared<omp::GlobalLightPanel>(m_GlobalLight);

    m_Widgets.push_back(std::make_shared<omp::MainLayer>());
    m_Widgets.push_back(m_RenderViewport);
    m_Widgets.push_back(std::move(entity));
    m_Widgets.push_back(std::move(material_panel));
    m_Widgets.push_back(m_ScenePanel);
    m_Widgets.push_back(std::move(camera_panel));
    m_Widgets.push_back(std::move(light_panel));
}

void Renderer::onViewportResize(size_t imageIndex)
{
    vkDestroyFramebuffer(m_LogicalDevice, m_SwapChainFramebuffers[imageIndex], nullptr);
    createFramebufferAtImage(imageIndex);
    createCommandBufferForImage(imageIndex);
}

void Renderer::createMaterialManager()
{
    m_VulkanContext->SetCommandPool(m_CommandPools);
    m_MaterialManager = std::make_unique<omp::MaterialManager>(m_VulkanContext);
}

