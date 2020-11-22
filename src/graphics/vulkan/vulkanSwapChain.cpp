#include "graphics/vulkan/vulkanSwapChain.hpp"
#include "graphics/vulkan/vulkanHelpers.hpp"
#include "graphics/vulkan/vulkanSwapChainSupportDetails.hpp"
#include "graphics/vulkan/vulkanQueueFamilyIndices.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

VkSurfaceFormatKHR vulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
            {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    {
                        return availableFormat;
                    }
            }

        return availableFormats[0];
    }

VkPresentModeKHR vulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
            {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                    {
                        return availablePresentMode;
                    }
            }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

vulkanSwapChain::vulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const GLFWwindow &window)
    {
        create(device, physicalDevice, surface, window);
    }

vulkanSwapChain::~vulkanSwapChain()
    {
        cleanup();
    }

void vulkanSwapChain::create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const GLFWwindow &window)
    {
        vulkanSwapChainSupportDetails swapChainSupport = helpers::querySwapChainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.m_formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.m_presentModes);

        if (swapChainSupport.m_capabilities.currentExtent.width != UINT32_MAX)
            {
                m_swapChainExtent = swapChainSupport.m_capabilities.currentExtent;
            }
        else
            {
                int width = 0;
                int height = 0;
                glfwGetFramebufferSize(const_cast<GLFWwindow*>(&window), &width, &height);

                m_swapChainExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

                m_swapChainExtent.width = std::max(swapChainSupport.m_capabilities.minImageExtent.width, std::min(swapChainSupport.m_capabilities.maxImageExtent.width, m_swapChainExtent.width));
                m_swapChainExtent.height = std::max(swapChainSupport.m_capabilities.minImageExtent.height, std::min(swapChainSupport.m_capabilities.maxImageExtent.height, m_swapChainExtent.height));
            }

        m_imageCount = swapChainSupport.m_capabilities.minImageCount + 1;

        if (swapChainSupport.m_capabilities.maxImageCount > 0 && m_imageCount > swapChainSupport.m_capabilities.maxImageCount)
            {
                m_imageCount = swapChainSupport.m_capabilities.maxImageCount;
            }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = m_imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = m_swapChainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vulkanQueueFamilyIndices indices = helpers::findQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = { indices.m_graphicsFamily.value(), indices.m_presentFamily.value() };

        if (indices.m_graphicsFamily != indices.m_presentFamily)
            {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
        else
            {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices = nullptr;
            }

        createInfo.preTransform = swapChainSupport.m_capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        vkGetSwapchainImagesKHR(device, m_swapChain, &m_imageCount, nullptr);
        m_swapChainImages.resize(m_imageCount);
        vkGetSwapchainImagesKHR(device, m_swapChain, &m_imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_associatedDevice = device;

        m_swapChainImageViews.resize(m_swapChainImages.size());
        for (std::size_t i = 0; i < m_swapChainImages.size(); i++)
            {
                m_swapChainImageViews[i].create(device, m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, VkImageViewType::VK_IMAGE_VIEW_TYPE_2D);
            }

        m_cleanedUp = false;
    }

void vulkanSwapChain::cleanup()
    {
        if (m_cleanedUp) { return; }
        for (auto &imageView : m_swapChainImageViews)
            {
                imageView.cleanup();
            }
        vkDestroySwapchainKHR(m_associatedDevice, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
        m_cleanedUp = true;
    }

const std::vector<vulkanImageView> &vulkanSwapChain::getImageViews() const
    {
        return m_swapChainImageViews;
    }

VkExtent2D vulkanSwapChain::getExtent() const
    {
        return m_swapChainExtent;
    }

VkFormat vulkanSwapChain::getFormat() const
    {
        return m_swapChainImageFormat;
    }

VkSwapchainKHR vulkanSwapChain::getUnderlyingSwapChain() const
    {
        return m_swapChain;
    }

VkSwapchainKHR &vulkanSwapChain::getUnderlyingSwapChain()
    {
        return m_swapChain;
    }

vulkanSwapChain::operator VkSwapchainKHR() const
    {
        return m_swapChain;
    }
