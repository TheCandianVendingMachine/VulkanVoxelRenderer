#include "graphics/vulkan/vulkanDescriptorPool.hpp"

vulkanDescriptorPool::vulkanDescriptorPool(VkDevice device, unsigned int descriptorSetCount, const std::vector<VkDescriptorPoolSize> &descriptors)
    {
        create(device, descriptorSetCount, descriptors);
    }

vulkanDescriptorPool::~vulkanDescriptorPool()
    {
        cleanup();
    }

void vulkanDescriptorPool::create(VkDevice device, unsigned int descriptorSetCount, const std::vector<VkDescriptorPoolSize> &descriptors)
    {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(descriptors.size());
        poolInfo.pPoolSizes = descriptors.data();
        poolInfo.maxSets = static_cast<uint32_t>(descriptorSetCount);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanDescriptorPool::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

bool vulkanDescriptorPool::isCreated() const
    {
        return !m_cleanedUp;
    }

VkDescriptorPool vulkanDescriptorPool::getUnderlyingDescriptorPool() const
    {
        return m_descriptorPool;
    }

vulkanDescriptorPool::operator VkDescriptorPool() const
    {
        return m_descriptorPool;
    }

VkDescriptorPool &vulkanDescriptorPool::getUnderlyingDescriptorPool()
    {
        return m_descriptorPool;
    }
