#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"

vulkanBuffer::vulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaMemoryUsage memoryUsage)
    {
        create(size, usage, properties, memoryUsage);
    }

vulkanBuffer::~vulkanBuffer()
    {
        cleanup();
    }

void vulkanBuffer::create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaMemoryUsage memoryUsage)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo bufferAllocInfo{};
        bufferAllocInfo.flags = 0;
        bufferAllocInfo.usage = memoryUsage;
        bufferAllocInfo.requiredFlags = properties;
        bufferAllocInfo.preferredFlags = 0;
        bufferAllocInfo.memoryTypeBits = 0;
        bufferAllocInfo.pool = VK_NULL_HANDLE;
        bufferAllocInfo.pUserData = nullptr;

        if (vmaCreateBuffer(*globals::g_vulkanAllocator, &bufferInfo, &bufferAllocInfo, &m_buffer, &m_allocation, nullptr) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_cleanedUp = false;
    }

void vulkanBuffer::cleanup()
    {
        if (m_cleanedUp) { return; }

        vmaDestroyBuffer(*globals::g_vulkanAllocator, m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VkBuffer vulkanBuffer::getUnderlyingBuffer() const
    {
        return m_buffer;
    }

vulkanBuffer::operator VkBuffer() const
    {
        return m_buffer;
    }

VmaAllocation vulkanBuffer::getUnderlyingAllocation() const
    {
        return m_allocation;
    }

vulkanBuffer::operator VmaAllocation() const
    {
        return m_allocation;
    }

VkBuffer &vulkanBuffer::getUnderlyingBuffer()
    {
        return m_buffer;
    }

VmaAllocation &vulkanBuffer::getUnderlyingAllocation()
    {
        return m_allocation;
    }
