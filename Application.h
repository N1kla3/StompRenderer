//
// Created by kolya on 8/18/2021.
//
#pragma once

#define NOMINMAX

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

namespace
{
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
             const VkAllocationCallbacks* pAllocator,
             VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

class Application {


    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool isComplete() const
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

public:
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

    void pickPhysicalDevice();

    void mainLoop();

    void cleanup();

    void createInstance();

    void createLogicalDevice();

    void createSurface();

    void createSwapChain();

    void createImageViews();

    bool checkValidationLayerSupport();

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void setupDebugMessenger();

    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    std::vector<const char*> getRequiredExtensions();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*  pUserData
            )
    {
        std::cerr << "validation layer" << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkSurfaceKHR m_Surface;

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    GLFWwindow* m_Window = nullptr;

    VkInstance m_Instance;

    VkPhysicalDevice m_PhysDevice;

    VkDevice m_LogicalDevice;

    VkSwapchainKHR m_SwapChain;

    VkQueue graphics_queue;

    VkQueue present_queue;

    std::vector<VkImage> m_SwapChainImages;

    std::vector<VkImageView> m_SwapChainImageViews;

    VkFormat m_SwapChainImageFormat;

    VkExtent2D m_SwapChainExtent;

    VkDebugUtilsMessengerEXT debugMessenger;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

};

