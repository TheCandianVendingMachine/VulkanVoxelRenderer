#include "voxel/heightmap.hpp"
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"
#include "graphics/renderer.hpp"
#include <stb_image.h>
#include <glm/common.hpp>

heightmap::heightmap(const char *filepath, renderer &renderer)
    {
        load(filepath, renderer);
    }

heightmap::~heightmap()
    {
        destroy();
    }

void heightmap::destroy()
    {
        m_heightmap.cleanup();
        m_heightmapSampler.cleanup();
        m_heightmapView.cleanup();
        if (m_pixelData)
            {
                stbi_image_free(m_pixelData);
                m_pixelData = nullptr;
            }
    }

void heightmap::load(const char *filepath, renderer &renderer)
    {
        m_pixelData = stbi_load(filepath, &m_size.x, &m_size.y, &m_channels, STBI_rgb_alpha);

        m_heightmap.create(m_size.x, m_size.y, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_TYPE_2D);

        unsigned int imageSize = m_size.x * m_size.y * 4;

        // buffer to copy to GPU memory
        vulkanBuffer buffer;
        buffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        void *data;
        vmaMapMemory(*globals::g_vulkanAllocator, buffer.getUnderlyingAllocation(), &data);
        std::memcpy(data, m_pixelData, static_cast<size_t>(imageSize));
        vmaUnmapMemory(*globals::g_vulkanAllocator, buffer.getUnderlyingAllocation());

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
            (uint32_t)m_size.x,
            (uint32_t)m_size.y,
            1
        };

        renderer.transitionImageLayout(m_heightmap, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copyBufferToImage(buffer, m_heightmap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        renderer.transitionImageLayout(m_heightmap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        buffer.cleanup();

        m_heightmapView.create(renderer.getDevice(), m_heightmap, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_heightmap.mipLevels, VK_IMAGE_VIEW_TYPE_2D);
        m_heightmapSampler.create(renderer.getDevice(), m_heightmap.mipLevels, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    }

vulkanImage &heightmap::getImage()
    {
        return m_heightmap;
    }

vulkanImageView &heightmap::getView()
    {
        return m_heightmapView;
    }

vulkanSampler &heightmap::getSampler()
    {
        return m_heightmapSampler;
    }

float heightmap::getHeight(glm::vec3 position)
    {
        const int a = 16;
        const int b = a * 64;

        glm::vec2 mapPos(position.x, position.z);
        mapPos /= a;

        if (mapPos.x < 0.f || mapPos.x >= m_size.x) { return 0.f; }
        if (mapPos.y < 0.f || mapPos.y >= m_size.y) { return 0.f; }

        glm::ivec2 pos = mapPos;

        unsigned int offset = static_cast<unsigned int>(pos.y * m_size.x + pos.x) * 4;
        unsigned char pixel = *(m_pixelData + offset);

        // desmos provided these numbers, pure magic
        return (static_cast<float>(pixel) / 255.f) * b;
    }
