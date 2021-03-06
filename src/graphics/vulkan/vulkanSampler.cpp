#include "graphics/vulkan/vulkanSampler.hpp"

vulkanSampler::vulkanSampler(VkDevice device, float maxLOD, VkSamplerAddressMode samplerMode)
    {
        create(device, maxLOD, samplerMode);
    }

vulkanSampler::~vulkanSampler()
    {
        cleanup();
    }

void vulkanSampler::create(VkDevice device, float maxLOD, VkSamplerAddressMode samplerMode)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = samplerMode;
        samplerInfo.addressModeV = samplerMode;
        samplerInfo.addressModeW = samplerMode;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 16.f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.f;
        samplerInfo.minLod = 0.f;
        samplerInfo.maxLod = maxLOD;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanSampler::cleanup()
    {
        if (m_cleanedUp) { return; }

        m_cleanedUp = true;
    }

bool vulkanSampler::isCreated() const
    {
        return !m_cleanedUp;
    }

VkSampler vulkanSampler::getUnderlyingSampler() const
    {
        return m_sampler;
    }

vulkanSampler::operator VkSampler() const
    {
        return m_sampler;
    }

VkSampler &vulkanSampler::getUnderlyingSampler()
    {
        return m_sampler;
    }
