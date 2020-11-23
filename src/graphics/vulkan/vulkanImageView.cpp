#include "graphics/vulkan/vulkanImageView.hpp"

vulkanImageView::vulkanImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int mipLevels, VkImageViewType imageType)
    {
        create(device, image, format, aspectFlags, mipLevels, imageType);
    }

vulkanImageView::~vulkanImageView()
    {
        cleanup();
    }

void vulkanImageView::create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int mipLevels, VkImageViewType imageType)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = imageType;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = static_cast<uint32_t>(mipLevels);
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanImageView::cleanup()
    {
        if (m_cleanedUp) { return; }
        vkDestroyImageView(m_device, m_imageView, nullptr);
        m_imageView = VK_NULL_HANDLE;
        m_cleanedUp = true;
    }

bool vulkanImageView::isCreated() const
    {
        return !m_cleanedUp;
    }

VkImageView vulkanImageView::getUnderlyingImageView() const
    {
        return m_imageView;
    }

vulkanImageView::operator VkImageView() const
    {
        return m_imageView;
    }

VkImageView &vulkanImageView::getUnderlyingImageView()
    {
        return m_imageView;
    }
