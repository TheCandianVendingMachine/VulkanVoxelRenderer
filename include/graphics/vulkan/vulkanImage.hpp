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
            int m_mipLevels;

            bool m_cleanedUp = true;

        public:
            vulkanImage();
            vulkanImage(unsigned int width, unsigned int height, unsigned int depth, int mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageType imageType);
            ~vulkanImage();
            void create(unsigned int width, unsigned int height, unsigned int depth, int mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkImageType imageType);
            void cleanup();

            bool isCreated() const;

            const int &mipLevels;

            VkImage getUnderlyingImage() const;
            operator VkImage() const;

            VkImage &getUnderlyingImage();

    };

