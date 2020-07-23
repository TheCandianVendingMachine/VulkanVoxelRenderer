#include "graphics/vulkan/vulkanImage.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"

vulkanImage::vulkanImage(unsigned int width, unsigned int height, int mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
    {
        create(width, height, mipLevels, numSamples, format, tiling, usage, memoryUsage);
    }

vulkanImage::~vulkanImage()
    {
        cleanup();
    }

void vulkanImage::create(unsigned int width, unsigned int height, int mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
    {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = static_cast<uint32_t>(width);
        imageCreateInfo.extent.height = static_cast<uint32_t>(height);
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = static_cast<uint32_t>(mipLevels);
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.samples = numSamples;
        imageCreateInfo.flags = 0;

        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.flags = 0;
        imageAllocInfo.usage = memoryUsage;
        imageAllocInfo.requiredFlags = 0;
        imageAllocInfo.preferredFlags = 0;
        imageAllocInfo.memoryTypeBits = 0;
        imageAllocInfo.pool = VK_NULL_HANDLE;
        imageAllocInfo.pUserData = nullptr;

        if (vmaCreateImage(*globals::g_vulkanAllocator, &imageCreateInfo, &imageAllocInfo, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_cleanedUp = false;
    }

void vulkanImage::cleanup()
    {
        if (m_cleanedUp) { return; }

        vmaDestroyImage(*globals::g_vulkanAllocator, m_image, m_allocation);
        m_image = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VkImage vulkanImage::getUnderlyingImage() const
    {
        return m_image;
    }

vulkanImage::operator VkImage() const
    {
        return m_image;
    }

VkImage &vulkanImage::getUnderlyingImage()
    {
        return m_image;
    }
