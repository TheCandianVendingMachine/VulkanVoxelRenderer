#include "graphics/vulkan/vulkanCommandPool.hpp"
#include <vk_mem_alloc.h>

vulkanCommandPool::vulkanCommandPool(VkDevice device, vulkanQueueFamilyIndices queueFamilyIndices)
    {
        create(device, queueFamilyIndices);
    }

vulkanCommandPool::~vulkanCommandPool()
    {
        cleanup();
    }

void vulkanCommandPool::create(VkDevice device, vulkanQueueFamilyIndices queueFamilyIndices)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanCommandPool::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VkCommandPool vulkanCommandPool::getUnderlyingCommandPool() const
    {
        return m_commandPool;
    }

vulkanCommandPool::operator VkCommandPool() const
    {
        return m_commandPool;
    }

VkCommandPool &vulkanCommandPool::getUnderlyingCommandPool()
    {
        return m_commandPool;
    }
