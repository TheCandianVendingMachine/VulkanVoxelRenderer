// vulkanSwapChain.hpp
// Wrapper around the Vulkan swap-chain
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "graphics/vulkan/vulkanImageView.hpp"

struct GLFWwindow;
class vulkanSwapChain
    {
        private:
            std::vector<VkImage> m_swapChainImages;
            std::vector<vulkanImageView> m_swapChainImageViews;

            VkDevice m_associatedDevice = VK_NULL_HANDLE;
            VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
            VkExtent2D m_swapChainExtent = {0, 0};
            VkFormat m_swapChainImageFormat = VkFormat::VK_FORMAT_UNDEFINED;
            uint32_t m_imageCount = 0;

            bool m_cleanedUp = true;

            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        public:
            vulkanSwapChain() = default;
            vulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const GLFWwindow &window);
            ~vulkanSwapChain();
            void create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const GLFWwindow &window);
            void cleanup();

            bool isCreated() const;

            const std::vector<vulkanImageView> &getImageViews() const;
            VkExtent2D getExtent() const;
            VkFormat getFormat() const;

            VkSwapchainKHR getUnderlyingSwapChain() const;
            VkSwapchainKHR &getUnderlyingSwapChain();
            operator VkSwapchainKHR() const;

    };
