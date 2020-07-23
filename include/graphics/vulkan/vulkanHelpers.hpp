// vulkanHelpers.hpp
// Miscellaneous helpers for various common vulkan functions
#pragma once
#include <vulkan/vulkan.h>
#include "graphics/vulkan/vulkanQueueFamilyIndices.hpp"
#include "graphics/vulkan/vulkanSwapChainSupportDetails.hpp"

namespace helpers
    {
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        vulkanQueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        vulkanSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
        VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    }
