//
// Created by kolya on 8/18/2021.
//
#pragma once

#define NOMINMAX

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>

#include <iostream>
#include "fstream"
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <optional>
#include "array"
#include <glm/glm.hpp>
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include "Scene.h"
#include "MaterialManager.h"
#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#include "LightObject.h"
#include "GraphicsPipeline.h"

namespace
{
    VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                               "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

namespace omp
{
    class ScenePanel;

    class ViewPort;
}

struct UniformBufferObject
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 view_position;
};

const std::string g_ModelPath = "../models/cube.obj";
const std::string g_TexturePath = "../textures/container.png";
const VkClearColorValue g_ClearColor = {0.52f, 0.48f, 0.52f, 1.0f};

class Renderer
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool IsComplete() const
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    // Methods //
    // ======= //
public:
    Renderer();

    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan();

    void initWindow();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    void pickPhysicalDevice();

    void mainLoop();

    void drawFrame();

    void cleanup();

    void createInstance();

    void createLogicalDevice();

    void createSurface();

    void createSwapChain();

    void createImageViews();

    void createRenderPass();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createFramebufferAtImage(size_t index);

    void createBuffer(
            VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

    void loadLightObject(const std::string& name, const std::string& textureName);

    std::shared_ptr<omp::Model> loadModel(const std::string& name, const std::string& modelName);

    void loadModelToBuffer(const omp::Model& model);

    void createUniformBuffers();

    void updateUniformBuffer(uint32_t currentImage);

    void createCommandBuffers();
    void createCommandBufferForImage(size_t index);

    void createCommandPool();

    void createDepthResources();

    void createImage(
            uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory,
            VkSampleCountFlagBits numSamples);

    void createTextureImage();

    void createColorResources();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    void createSyncObjects();

    void createDescriptorSetLayout();
    void createDescriptorPool();

    void createDescriptorSets();
    void createDescriptorSetsForMaterial(const std::shared_ptr<omp::Material>& material);

    void recreateSwapChain();

    void cleanupSwapChain();

    void createImguiContext();
    void initializeImgui();
    void createImguiRenderPass();

    void createImguiCommandPools();
    void createImguiCommandBuffers();
    void createImguiCommandBufferAtIndex(uint32_t imageIndex);
    void renderAllUi();
    void createImguiWidgets();
    void createImguiFramebuffers();

    void createMaterialManager();

    void onViewportResize(size_t imageIndex);

    bool checkValidationLayerSupport();

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void setupDebugMessenger();

    bool isDeviceSuitable(VkPhysicalDevice device);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void transitionImageLayout(
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    std::vector<const char*> getRequiredExtensions();

    VkSampleCountFlagBits getMaxUsableSampleCount();

    omp::GraphicsPipeline* findGraphicsPipeline(const std::string& name);

    void createVertexBufferAndMemoryAtIndex(size_t index);
    void createIndexBufferAndMemoryAtIndex(size_t index);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
    )
    {
        std::cerr << "validation layer" << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file");
        }

        size_t file_size = (size_t) file.tellg();
        std::vector<char> buffer(file_size);
        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();
        return buffer;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    // State //
    // ===== //
    VkSurfaceKHR m_Surface;

    GLFWwindow* m_Window = nullptr;

    VkInstance m_Instance;

    VkPhysicalDevice m_PhysDevice;

    VkDevice m_LogicalDevice;

    VkSwapchainKHR m_SwapChain;

    VkQueue m_GraphicsQueue;

    VkQueue m_PresentQueue;

    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkRenderPass m_RenderPass;

    std::unordered_map<std::string, std::unique_ptr<omp::GraphicsPipeline>> m_Pipelines;

    VkCommandPool m_CommandPools;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;

    std::unordered_map<omp::Vertex, uint32_t> m_UniqueVertices;

    std::vector<VkBuffer> m_VertexBuffers;
    std::vector<VkDeviceMemory> m_VertexBufferMemories;

    std::vector<VkBuffer> m_IndexBuffers;
    std::vector<VkDeviceMemory> m_IndexBufferMemories;

    std::shared_ptr<omp::Material> m_DefaultMaterial;

    VkImage m_ColorImage;
    VkDeviceMemory m_ColorImageMemory;
    VkImageView m_ColorImageView;

    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;

    std::vector<VkBuffer> m_LightBuffer;
    std::vector<VkDeviceMemory> m_LightBufferMemory;

    std::vector<VkImage> m_SwapChainImages;

    std::vector<VkImageView> m_SwapChainImageViews;

    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    std::vector<VkCommandBuffer> m_CommandBuffers;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;

    std::vector<VkFence> m_InFlightFences;
    std::vector<VkFence> m_ImagesInFlight;

    VkImage m_DepthImage;
    VkDeviceMemory m_DepthImageMemory;
    VkImageView m_DepthImageView;

    std::shared_ptr<omp::Scene> m_CurrentScene;
    // Todo: maybe multiple later
    // todo: event driven
    std::shared_ptr<omp::ViewPort> m_RenderViewport;
    std::shared_ptr<omp::ScenePanel> m_ScenePanel;

    std::shared_ptr<omp::Camera> m_Camera;
    std::shared_ptr<omp::Light> m_GlobalLight;

    std::shared_ptr<omp::LightObject> m_LightObject;

    std::vector<std::shared_ptr<omp::ImguiUnit>> m_Widgets;

    VkFormat m_SwapChainImageFormat;

    VkExtent2D m_SwapChainExtent;

    size_t m_CurrentFrame = 0;

    bool m_FramebufferResized = false;

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    VkRenderPass m_ImguiRenderPass;
    VkCommandPool m_ImguiCommandPool;
    std::vector<VkCommandBuffer> m_ImguiCommandBuffers;
    std::vector<VkFramebuffer> m_ImguiFramebuffers;
    VkDescriptorPool m_ImguiDescriptorPool;

    std::shared_ptr<omp::VulkanContext> m_VulkanContext;

    VkSampleCountFlagBits m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    const uint32_t WIDTH = 1920;
    const uint32_t HEIGHT = 1080;
    const int MAX_FRAMES_IN_FLIGHT = 2;

};

