#include "graphics/image.hpp"

void image::create(VkDevice device, VkImageAspectFlagBits imageAspectFlags, unsigned int width, unsigned int height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
    {
        m_image.create(width, height, mipLevels, numSamples, format, tiling, usage, memoryUsage);
        m_imageView.create(device, m_image, format, imageAspectFlags, mipLevels);
        setImageSize(width, height);
        setMipLevels(mipLevels);
    }

void image::cleanup()
    {
        m_imageView.cleanup();
        m_image.cleanup();
    }

const vulkanImage &image::getImage() const
    {
        return m_image;
    }

const vulkanImageView &image::getImageView() const
    {
        return m_imageView;
    }

vulkanImage &image::getImage()
    {
        return m_image;
    }

vulkanImageView &image::getImageView()
    {
        return m_imageView;
    }

void image::setMipLevels(uint32_t levels)
    {
        m_mipLevels = levels;
    }

uint32_t image::getMipLevels() const
    {
        return m_mipLevels;
    }

void image::setImageSize(int width, int height)
    {
        m_textureWidth = width;
        m_textureHeight = height;
    }

glm::uvec2 image::getImageSize() const
    {
        return glm::uvec2(m_textureWidth, m_textureHeight);
    }
