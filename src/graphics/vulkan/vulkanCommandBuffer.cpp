#include "graphics/vulkan/vulkanCommandBuffer.hpp"

vulkanCommandBuffer::vulkanCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        create(device, commandPool, level);
    }

vulkanCommandBuffer::~vulkanCommandBuffer()
    {
        cleanup();
    }

void vulkanCommandBuffer::create(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = level;
        allocInfo.commandBufferCount = static_cast<uint32_t>(1);

        if (vkAllocateCommandBuffers(device, &allocInfo, &m_commandBuffer) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanCommandBuffer::cleanup()
    {
        if (m_cleanedUp) { return; }

        m_cleanedUp = true;
    }

VkCommandBuffer vulkanCommandBuffer::getUnderlyingCommandBuffer() const
    {
        return m_commandBuffer;
    }

vulkanCommandBuffer::operator VkCommandBuffer() const
    {
        return m_commandBuffer;
    }

VkCommandBuffer &vulkanCommandBuffer::getUnderlyingCommandBuffer()
    {
        return m_commandBuffer;
    }

void vulkanCommandBufferFunctions::createBatch(std::vector<vulkanCommandBuffer> &commandBuffers, VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        std::vector<VkCommandBuffer> commandBuffersRaw(commandBuffers.begin(), commandBuffers.end());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = level;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffersRaw.data()) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        unsigned int i = 0;
        for (auto &commandBuffer : commandBuffers)
            {
                commandBuffer.m_commandBuffer = commandBuffersRaw[i++];
                commandBuffer.m_device = device;
                commandBuffer.m_cleanedUp = false;
            }
    }
