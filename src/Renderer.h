#pragma once

#define NOMINMAX

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL


#include <iostream>
#include "fstream"
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <optional>
#include "array"
#include <glm/glm.hpp>
#include <queue>
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include "Scene.h"
#include "MaterialManager.h"
#include "Rendering/Shader.h"
#include "Camera.h"
#include "Rendering/GraphicsPipeline.h"
#include "Rendering/RenderPass.h"
#include "Rendering/FrameBuffer.h"
#include "UI/ImguiUnit.h"
#include "Logs.h"
#include "LightSystem.h"
#include "Rendering/ModelStatics.h"

namespace
{
    VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
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
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
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

    struct UniformBufferObject
    {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 view_position;

        // TODO: check shaders to have same type
        uint32_t global_light_enabled;
        uint32_t point_light_size;
        uint32_t spot_light_size;
    };

    struct OutlineUniformBuffer
    {
        glm::mat4 model;
        glm::mat4 projection;
        glm::mat4 view;
    };

    struct CommandBufferScope
    {
        VkCommandBuffer buffer;
        bool is_allocated = false;


        void clearBuffer(VkDevice device, VkCommandPool pool)
        {
            if (!is_allocated)
            {
                VWARN(LogRenderer, "Cant free already cleared command buffer");
                return;
            }
            vkFreeCommandBuffers(device, pool, 1, &buffer);
            is_allocated = false;
        }

        void reAllocate(VkDevice device, VkCommandPool pool, VkCommandBufferAllocateInfo bufferInfo)
        {
            if (is_allocated)
            {
                vkFreeCommandBuffers(device, pool, 1, &buffer);
            }
            vkAllocateCommandBuffers(device, &bufferInfo, &buffer);
            is_allocated = true;
        }
    };

    const std::string g_ModelPath = "../models/cube2.obj";
    const std::string g_TexturePath = "../textures/container.png";
    const VkClearColorValue g_ClearColor = {0.82f, 0.48f, 0.52f, 1.0f};

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

        void initVulkan(GLFWwindow* window);
        void initResources();
        void loadScene(omp::Scene* scene);
        // TODO: void initNewScene(omp::Scene* scene);
        
        void requestDrawFrame(float deltaTime);

            //TODO: initWindow();
            // TODO: initVulkan();
            //initializeScene();
            // TODO: requestDrawFrame();
            //cleanup();

        void onWindowResize(int width, int height);

    private:

        void pickPhysicalDevice();

        void drawFrame();
        void initializeScene();
        void postFrame();
        void tick(float deltaTime);

        void cleanup();
        void destroyAllCommandBuffers();

        void createInstance();

        void createLogicalDevice();

        void createSurface(GLFWwindow* window);

        void createSwapChain();

        void postSwapChainInitialize();
        
        void prepareSceneForRendering();

        void createImageViews();

        void createRenderPass();

        void createGraphicsPipeline();

        void createFramebuffers();

        void createFramebufferAtImage(size_t index);


        void prepareCommandBuffer(CommandBufferScope& bufferScope, VkCommandPool inCommandPool);
        void setViewport(VkCommandBuffer inCommandBuffer);
        void beginRenderPass(
                omp::RenderPass* inRenderPass,
                VkCommandBuffer inCommandBuffer,
                omp::FrameBuffer& inFrameBuffer,
                const std::vector<VkClearValue>& clearValues,
                VkRect2D rect = VkRect2D());
        void endRenderPass(omp::RenderPass* inRenderPass, VkCommandBuffer inCommandBuffer);

        void createUniformBuffers();

        void updateUniformBuffer(uint32_t currentImage);

        void prepareFrameForImage(size_t KHRImageIndex);

        void createCommandPool();

        void createDepthResources();

        void createColorResources();
        void createViewportResources();
        void createPickingResources();

        void createSyncObjects();

        void createDescriptorSetLayout();
        void createDescriptorPool();

        void createDescriptorSets();

        void addModelToScene(std::unique_ptr<omp::SceneEntity>&& inModel);
        std::shared_ptr<omp::ModelInstance> addModelToScene(const std::string& inName, const std::string& inPath);
        void loadModelInMemory(const std::shared_ptr<omp::Model>& inModel);

        void retrieveMaterialRenderState(const std::shared_ptr<omp::Material>& material);

        void recreateSwapChain();

        void cleanupSwapChain();

        void createImguiContext();
        void initializeImgui(GLFWwindow* window);
        void createImguiRenderPass();

        void createImguiCommandPools();
        void renderAllUi();
        void createImguiWidgets();
        void createImguiFramebuffers();

        void createMaterialManager();
        void createLights();

        void destroyMainRenderPassResources();
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

        VkFormat findSupportedFormat(
                const std::vector<VkFormat>& candidates,
                VkImageTiling tiling,
                VkFormatFeatureFlags features);
        VkFormat findDepthFormat();

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        std::vector<const char*> getRequiredExtensions();

        VkSampleCountFlagBits getMaxUsableSampleCount();

        omp::GraphicsPipeline* findGraphicsPipeline(const std::string& name);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* /*pUserData*/
        )
        {
            if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                WARN(LogRendering, "Validation layer{}", pCallbackData->pMessage);
            }
            else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                ERROR(LogRendering, "Validation layer{}", pCallbackData->pMessage);
            }
            else
            {
                INFO(LogRendering, "Validation layer{}", pCallbackData->pMessage);
            }
            return VK_FALSE;
        }

        static std::vector<char> readFile(const std::string& filename)
        {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open())
            {
                ERROR(LogRendering, "Failed to open file: {}", filename);
                throw std::runtime_error("Failed to open file");
            }

            size_t file_size = static_cast<size_t>(file.tellg());
            std::vector<char> buffer(file_size);
            file.seekg(0);
            file.read(buffer.data(), static_cast<int32_t>(file_size));

            file.close();
            return buffer;
        }

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        // State //
        // ===== //
        VkSurfaceKHR m_Surface;

        VkInstance m_Instance;

        VkPhysicalDevice m_PhysDevice;
        VkPhysicalDeviceLimits m_DeviceLimits;

        VkDevice m_LogicalDevice;

        VkSwapchainKHR m_SwapChain;

        VkQueue m_GraphicsQueue;

        VkQueue m_PresentQueue;

        uint32_t m_PresentKHRImagesNum;

        VkDescriptorSetLayout m_UboDescriptorSetLayout;
        VkDescriptorSetLayout m_TexturesDescriptorSetLayout;
        VkDescriptorSetLayout m_OutlineSetLayout;
        VkDescriptorSetLayout m_SkyboxSetLayout;

        std::shared_ptr<omp::RenderPass> m_RenderPass;

        std::unordered_map<std::string, std::unique_ptr<omp::GraphicsPipeline>> m_Pipelines;

        VkCommandPool m_CommandPool;
        VkDescriptorPool m_DescriptorPool;
        std::vector<VkDescriptorSet> m_UboDescriptorSets;
        std::vector<VkDescriptorSet> m_MaterialSets;
        std::vector<VkDescriptorSet> m_OutlineDescriptorSets;
        std::vector<VkDescriptorSet> m_SkyboxDescriptorSets;

        std::shared_ptr<omp::Material> m_DefaultMaterial;

        VkImage m_ColorImage;
        VkDeviceMemory m_ColorImageMemory;
        VkImageView m_ColorImageView;

        VkImage m_ViewportImage;
        VkImageView m_ViewportImageView;
        VkSampler m_ViewportSampler = VK_NULL_HANDLE;
        VkDeviceMemory m_ViewportImageMemory;
        VkDescriptorSet m_ViewportDescriptor = VK_NULL_HANDLE;

        VkImage m_PickingImage;
        VkImageView m_PickingImageView;
        VkDeviceMemory m_PickingMemory;
        VkImage m_PickingResolve;
        VkImageView m_PickingResolveView;
        VkDeviceMemory m_PickingResolveMemory;

        VkBuffer m_PixelReadBuffer;
        VkDeviceMemory m_PixelReadMemory;

        std::unique_ptr<omp::UniformBuffer> m_UboBuffer;
        std::unique_ptr<omp::UniformBuffer> m_OutlineBuffer;

        std::vector<VkImage> m_SwapChainImages;

        std::vector<VkImageView> m_SwapChainImageViews;

        std::vector<omp::FrameBuffer> m_SwapChainFramebuffers;

        std::vector<CommandBufferScope> m_CommandBuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;

        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;

        VkImage m_DepthImage;
        VkDeviceMemory m_DepthImageMemory;
        VkImageView m_DepthImageView;

        omp::Scene* m_CurrentScene;
        std::shared_ptr<omp::ViewPort> m_RenderViewport;
        std::shared_ptr<omp::ScenePanel> m_ScenePanel;

        GLFWwindow* m_Window;

        std::unique_ptr<omp::LightSystem> m_LightSystem;

        std::vector<std::shared_ptr<omp::ImguiUnit>> m_Widgets;

        VkFormat m_SwapChainImageFormat;

        VkExtent2D m_SwapChainExtent;

        size_t m_CurrentFrame = 0;

        bool m_FramebufferResized = false;

        VkDebugUtilsMessengerEXT m_DebugMessenger;

        std::shared_ptr<omp::RenderPass> m_ImguiRenderPass;
        VkCommandPool m_ImguiCommandPool;
        std::vector<CommandBufferScope> m_ImguiCommandBuffers;
        std::vector<omp::FrameBuffer> m_ImguiFramebuffers;
        VkDescriptorPool m_ImguiDescriptorPool;

        std::shared_ptr<omp::VulkanContext> m_VulkanContext;

        std::queue<ImVec2> m_MousePickingData{};

        VkSampleCountFlagBits m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        int m_CurrentWidth = 0;
        int m_CurrentHeight = 0;
        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    };
}
