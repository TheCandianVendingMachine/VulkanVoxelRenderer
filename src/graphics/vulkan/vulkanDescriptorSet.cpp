#include "graphics/vulkan/vulkanDescriptorSet.hpp"

vulkanDescriptorSet::vulkanDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts)
    {
        create(device, descriptorPool, descriptorSetLayouts);
    }

vulkanDescriptorSet::~vulkanDescriptorSet()
    {
        cleanup();
    }

void vulkanDescriptorSet::create(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = descriptorSetLayouts.data();
        if (vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanDescriptorSet::cleanup()
    {
        if (m_cleanedUp) { return; }

        m_cleanedUp = true;
    }

VkDescriptorSet vulkanDescriptorSet::getUnderlyingDescriptorSet() const
    {
        return m_descriptorSet;
    }

vulkanDescriptorSet::operator VkDescriptorSet() const
    {
        return m_descriptorSet;
    }

VkDescriptorSet &vulkanDescriptorSet::getUnderlyingDescriptorSet()
    {
        return m_descriptorSet;
    }

void vulkanDescriptorSetFunctions::createBatch(std::vector<vulkanDescriptorSet> &descriptorSets, VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts)
    {
        std::vector<VkDescriptorSet> descriptorSetsRaw(descriptorSets.begin(), descriptorSets.end());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSets.size());
        allocInfo.pSetLayouts = descriptorSetLayouts.data();
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsRaw.data()) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        unsigned int i = 0;
        for (auto &descriptorSet : descriptorSets)
            {
                descriptorSet.m_descriptorSet = descriptorSetsRaw[i++];
                descriptorSet.m_device = device;
                descriptorSet.m_cleanedUp = false;
            }
    }
