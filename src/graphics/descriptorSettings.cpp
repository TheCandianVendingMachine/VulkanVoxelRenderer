#include "graphics/descriptorSettings.hpp"

void descriptorSettings::addSetting(VkDescriptorType type, VkShaderStageFlags stages, unsigned int count)
    {
        m_descriptorSettings.push_back({ type, stages, count, static_cast<unsigned int>(m_descriptorSettings.size()) });
    }

std::vector<VkDescriptorSetLayoutBinding> descriptorSettings::getLayoutBindings() const
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(m_descriptorSettings.size());
        for (unsigned int i = 0; i < m_descriptorSettings.size(); i++)
            {
                layoutBindings[i].binding = i;
                layoutBindings[i].descriptorType = m_descriptorSettings[i].m_type;
                layoutBindings[i].descriptorCount = m_descriptorSettings[i].m_descriptorCount;
                layoutBindings[i].stageFlags = m_descriptorSettings[i].m_shaderStages;
                layoutBindings[i].pImmutableSamplers = nullptr;
            }

        return layoutBindings;
    }

std::vector<VkDescriptorPoolSize> descriptorSettings::getPoolSizes(unsigned int swapChainImageCount) const
    {
        std::vector<VkDescriptorPoolSize> poolSizes(m_descriptorSettings.size());

        for (unsigned int i = 0; i < m_descriptorSettings.size(); i++)
            {
                poolSizes[i].type = m_descriptorSettings[i].m_type;
                poolSizes[i].descriptorCount = swapChainImageCount;
            }

        return poolSizes;
    }

std::vector<VkWriteDescriptorSet> descriptorSettings::getDescriptorWrites() const
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites(m_descriptorSettings.size());

        for (int i = 0; i < m_descriptorSettings.size(); i++)
            {
                descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[i].dstBinding = m_descriptorSettings[i].m_bindingNumber;
                descriptorWrites[i].dstArrayElement = 0;
                descriptorWrites[i].descriptorType = m_descriptorSettings[i].m_type;
                descriptorWrites[i].descriptorCount = m_descriptorSettings[i].m_descriptorCount;
                descriptorWrites[i].pBufferInfo = nullptr;
                descriptorWrites[i].pImageInfo = nullptr;
                descriptorWrites[i].pTexelBufferView = nullptr;
                descriptorWrites[i].pNext = nullptr;
            }

        return descriptorWrites;
    }
