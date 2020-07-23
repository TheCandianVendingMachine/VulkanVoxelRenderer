// vulkanImage.hpp
// A wrapper around VkImage. Handles its own memory so not a pure wrapper
#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class vulkanImage
    {
        private:
            VkImage m_image = VK_NULL_HANDLE;
            VmaAllocation m_allocation = VK_NULL_HANDLE;

            bool m_cleanedUp = false;

        public:
            vulkanImage() = default;
            vulkanImage(unsigned int width, unsigned int height, int mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
            ~vulkanImage();
            void create(unsigned int width, unsigned int height, int mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
            void cleanup();

            VkImage getUnderlyingImage() const;
            operator VkImage() const;

            VkImage &getUnderlyingImage();

    };

