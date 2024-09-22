#pragma once

#include "Rendering/VulkanImage.h"
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
#include <glm/glm.hpp>
#include "imgui.h"

#include "Scene.h"
#include "Rendering/GraphicsPipeline.h"
#include "Rendering/RenderPass.h"
#include "Rendering/FrameBuffer.h"
#include "Logs.h"
#include "LightSystem.h"

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

        void initVulkan(GLFWwindow* window, int initWidth, int initHeight);
        void initResources();
        void loadScene(omp::Scene* scene);
        // TODO: void initNewScene(omp::Scene* scene);

        bool prepareFrame();
        void requestDrawFrame(float deltaTime);
        void resizeViewport(uint32_t x, uint32_t y);
        void setClickedEntity(uint32_t x, uint32_t y);

        void onWindowResize(int width, int height);
        void cleanup();
        VkDescriptorSet getViewportDescriptor() { return m_ViewportImage->getImguiImage(); }

    private:

        void resizeInternal();
        void pickPhysicalDevice();

        void drawFrame();
        void tick(float deltaTime);

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

        void retrieveMaterialRenderState(const std::shared_ptr<omp::Material>& material);

        void recreateSwapChain();

        void cleanupSwapChain();

        void createImguiContext();
        void initializeImgui(GLFWwindow* window);
        void createImguiRenderPass();

        void createImguiCommandPools();
        void createImguiFramebuffers();

        void destroyMainRenderPassResources();

        bool checkValidationLayerSupport();

        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& /*capabilities*/);

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
        // TODO: reconsider approach of materials
        std::vector<VkDescriptorSet> m_MaterialSets;
        std::vector<VkDescriptorSet> m_OutlineDescriptorSets;
        std::vector<VkDescriptorSet> m_SkyboxDescriptorSets;

        std::shared_ptr<omp::Material> m_DefaultMaterial;

        VkImage m_ColorImage;
        VkDeviceMemory m_ColorImageMemory;
        VkImageView m_ColorImageView;

        std::unique_ptr<omp::VulkanImage> m_ViewportImage = nullptr;

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

        omp::Scene* m_CurrentScene = nullptr;

        GLFWwindow* m_Window;

        std::unique_ptr<omp::LightSystem> m_LightSystem;

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

        VkSampleCountFlagBits m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        uint32_t m_ViewportSize[2]{110,110};
        uint32_t m_RequestedViewportSize[2]{110,110};
        bool m_ShouldResize = false;
        int m_CurrentWidth = 0;
        int m_CurrentHeight = 0;
        uint32_t m_CurrentImage = 0;
        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    };
}
