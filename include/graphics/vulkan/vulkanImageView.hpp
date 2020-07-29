// vulkanImageView.hpp
// A wrapper around VkImageView
#pragma once
#include <vulkan/vulkan.h>

class vulkanImageView
    {
        private:
            VkImageView m_imageView = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;
            bool m_cleanedUp = true;

        public:
            vulkanImageView() = default;
            vulkanImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int mipLevels);
            ~vulkanImageView();
            void create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int mipLevels);
            void cleanup();

            VkImageView getUnderlyingImageView() const;
            operator VkImageView() const;

            VkImageView &getUnderlyingImageView();

    };
