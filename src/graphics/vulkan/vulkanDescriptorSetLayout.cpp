#include "graphics/vulkan/vulkanDescriptorSetLayout.hpp"

vulkanDescriptorSetLayout::vulkanDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &layoutBindings)
    {
        create(device, layoutBindings);
    }

vulkanDescriptorSetLayout::~vulkanDescriptorSetLayout()
    {
        cleanup();
    }

void vulkanDescriptorSetLayout::create(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &layoutBindings)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanDescriptorSetLayout::cleanup()
    {
        if (m_cleanedUp) { return; }
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
        m_descriptorSetLayout = VK_NULL_HANDLE;
        m_cleanedUp = true;
    }

VkDescriptorSetLayout vulkanDescriptorSetLayout::getUnderlyingDescriptorSetLayout() const
    {
        return m_descriptorSetLayout;
    }

vulkanDescriptorSetLayout::operator VkDescriptorSetLayout() const
    {
        return m_descriptorSetLayout;
    }

VkDescriptorSetLayout &vulkanDescriptorSetLayout::getUnderlyingDescriptorSetLayout()
    {
        return m_descriptorSetLayout;
    }
