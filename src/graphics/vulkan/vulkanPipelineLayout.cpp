#include "graphics/vulkan/vulkanPipelineLayout.hpp"

vulkanPipelineLayout::vulkanPipelineLayout(VkDevice device, const std::vector<VkDescriptorSetLayout> &layoutInfo)
    {
        create(device, layoutInfo);
    }

vulkanPipelineLayout::~vulkanPipelineLayout()
    {
        cleanup();
    }

void vulkanPipelineLayout::create(VkDevice device, const std::vector<VkDescriptorSetLayout> &layoutInfo)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layoutInfo.size());
        pipelineLayoutInfo.pSetLayouts = layoutInfo.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanPipelineLayout::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VkPipelineLayout vulkanPipelineLayout::getUnderlyingPipelineLayout() const
    {
        return m_pipelineLayout;
    }

vulkanPipelineLayout::operator VkPipelineLayout() const
    {
        return m_pipelineLayout;
    }

VkPipelineLayout &vulkanPipelineLayout::getUnderlyingPipelineLayout()
    {
        return m_pipelineLayout;
    }
