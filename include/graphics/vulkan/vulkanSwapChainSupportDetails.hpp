// swapChainSupportDetails.hpp
// Struct to contain details on whether or not a swapchain supports the required features
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

struct vulkanSwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR m_capabilities;
        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkPresentModeKHR> m_presentModes;
    };
