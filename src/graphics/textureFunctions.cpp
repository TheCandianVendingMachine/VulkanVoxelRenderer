#include "graphics/textureFunctions.hpp"
#include "graphics/vulkan/vulkanImage.hpp"
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/renderer.hpp"
#include <stb_image.h>

textureData loadTexture(const char *filepath, vulkanImage &image, renderer &renderer)
    {
        textureData textureData;
        stbi_uc *m_pixelData = stbi_load(filepath, &textureData.width, &textureData.height, &textureData.channels, STBI_rgb_alpha);

        if (!image.isCreated())
            {
                image.create(textureData.width, textureData.height, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_TYPE_2D);
            }

        unsigned int imageSize = textureData.width * textureData.height * 4;
        vulkanBuffer buffer;
        buffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        void *data;
        vmaMapMemory(*globals::g_vulkanAllocator, buffer.getUnderlyingAllocation(), &data);
        std::memcpy(data, m_pixelData, static_cast<size_t>(imageSize));
        vmaUnmapMemory(*globals::g_vulkanAllocator, buffer.getUnderlyingAllocation());            

        stbi_image_free(m_pixelData);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            (uint32_t)textureData.width,
            (uint32_t)textureData.height,
            1
        };

        renderer.transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copyBufferToImage(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        renderer.transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        buffer.cleanup();

        return textureData;
    }
