// image.hpp
// Defines an image within memory.
#pragma once
#include <glm/vec2.hpp>
#include "graphics/vulkan/vulkanImage.hpp"
#include "graphics/vulkan/vulkanImageView.hpp"

class image
    {
        protected:
            vulkanImage m_image;
            vulkanImageView m_imageView;
            uint32_t m_mipLevels = 0;

            int m_textureWidth = 0;
            int m_textureHeight = 0;

            virtual void cleanupInternal() {}

        public:
            void create(VkDevice device, VkImageAspectFlagBits imageAspectFlags, unsigned int width, unsigned int height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);
            void cleanup();

            const vulkanImage &getImage() const;
            const vulkanImageView &getImageView() const;

            vulkanImage &getImage();
            vulkanImageView &getImageView();

            void setMipLevels(uint32_t levels);
            uint32_t getMipLevels() const;

            void setImageSize(int width, int height);
            glm::uvec2 getImageSize() const;
    };
