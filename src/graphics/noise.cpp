#include "graphics/noise.hpp"
#include "graphics/renderer.hpp"
#include "graphics/storageBuffer.hpp"
#include <vector>

void noise::generateTexture(bool threeDimensions, renderer &renderer)
    {
        VkImageType imageType = VkImageType::VK_IMAGE_TYPE_2D;
        VkImageViewType viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
        if (threeDimensions)
            {
                imageType = VkImageType::VK_IMAGE_TYPE_3D;
                viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
            }

        VkFormat format = VK_FORMAT_R32_SFLOAT;
        m_noiseImage.create(m_noiseTextureSize.x, m_noiseTextureSize.y, m_noiseTextureSize.z, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, imageType);
        m_noiseView.create(renderer.getDevice(), m_noiseImage, format, VK_IMAGE_ASPECT_COLOR_BIT, m_noiseImage.mipLevels, viewType);
        m_noiseSampler.create(renderer.getDevice(), m_noiseImage.mipLevels, VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT);

        std::vector<float> noiseData;
        for (unsigned int z = 0; z < m_noiseTextureSize.z; z++)
            {
                for (unsigned int y = 0; y < m_noiseTextureSize.y; y++)
                    {
                        for (unsigned int x = 0; x < m_noiseTextureSize.x; x++)
                            {
                                float data = sample(x, y, z);
                                noiseData.push_back(data);
                            }
                    }
            }

        storageBuffer temp;
        temp.create(noiseData.size(), sizeof(float), noiseData.data());

        VkBufferImageCopy bufferCopyRegion{};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.mipLevel = 0;
        bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = m_noiseTextureSize.x;
        bufferCopyRegion.imageExtent.height = m_noiseTextureSize.y;
        bufferCopyRegion.imageExtent.depth = m_noiseTextureSize.z;

        renderer.transitionImageLayout(m_noiseImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copyBufferToImage(temp.getStorageBuffer(), m_noiseImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
        renderer.transitionImageLayout(m_noiseImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        temp.destroy();
    }

noise::noise()
    {
        m_noiseGenerator.SetNoiseType(FastNoise::NoiseType::WhiteNoise);
    }

noise::noise(FastNoise::NoiseType noiseType, FN_DECIMAL frequency, FN_DECIMAL lacunarity, FN_DECIMAL gain, int octaves)
    {
        init(noiseType, frequency, lacunarity, gain, octaves);
    }

void noise::init(FastNoise::NoiseType noiseType, FN_DECIMAL frequency, FN_DECIMAL lacunarity, FN_DECIMAL gain, int octaves)
    {
        m_noiseGenerator.SetNoiseType(noiseType);
        m_noiseGenerator.SetFrequency(frequency);
        m_noiseGenerator.SetFractalLacunarity(lacunarity);
        m_noiseGenerator.SetFractalGain(gain);
        m_noiseGenerator.SetFractalOctaves(octaves);
    }

noise::~noise()
    {
        destroy();
    }

void noise::destroy()
    {
        m_noiseImage.cleanup();
        m_noiseSampler.cleanup();
        m_noiseView.cleanup();
    }

void noise::createTexture(glm::uvec2 size, renderer &renderer)
    {
        m_noiseTextureSize = { size.x, size.y, 1 };
        generateTexture(false, renderer);
    }

void noise::createTexture(glm::uvec3 size, renderer &renderer)
    {
        m_noiseTextureSize = size;
        generateTexture(true, renderer);
    }

FN_DECIMAL noise::sample(FN_DECIMAL x, FN_DECIMAL y) const
    {
        return m_noiseGenerator.GetNoise(x, y);
    }

FN_DECIMAL noise::sample(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const
    {
        return m_noiseGenerator.GetNoise(x, y, z);
    }

const vulkanImage &noise::getImage() const
    {
        return m_noiseImage;
    }

const vulkanSampler &noise::getSampler() const
    {
        return m_noiseSampler;
    }

const vulkanImageView &noise::getView() const
    {
        return m_noiseView;
    }
