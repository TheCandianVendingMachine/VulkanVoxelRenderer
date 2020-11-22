#include "graphics/storageBuffer.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"
#include <cstring>

storageBuffer::storageBuffer(unsigned int count, std::size_t objectSize)
    {
        create(count, objectSize);
    }

storageBuffer::storageBuffer(unsigned int count, std::size_t objectSize, void *data)
    {
        create(count, objectSize, data);
    }

storageBuffer::~storageBuffer()
    {
        destroy();
    }

void storageBuffer::create(unsigned int count, std::size_t objectSize)
    {
        m_storageBuffer.create(count * objectSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        m_count = count;
        m_size = objectSize;
    }

void storageBuffer::create(unsigned int count, std::size_t objectSize, void *data)
    {
        create(count, objectSize);
        bind(data);
    }

void storageBuffer::destroy()
    {
        m_storageBuffer.cleanup();
    }

void storageBuffer::bind(void *data)
    {
        void *tempData = nullptr;
        vmaMapMemory(*globals::g_vulkanAllocator, m_storageBuffer, &tempData);
        std::memcpy(tempData, data, m_size * m_count);
        vmaUnmapMemory(*globals::g_vulkanAllocator, m_storageBuffer);
    }

const vulkanBuffer &storageBuffer::getStorageBuffer() const
    {
        return m_storageBuffer;
    }

vulkanBuffer &storageBuffer::getStorageBuffer()
    {
        return m_storageBuffer;
    }

unsigned int storageBuffer::getBufferCount() const
    {
        return m_count;
    }

std::size_t storageBuffer::getBufferObjectSize() const
    {
        return m_size;
    }

VkDeviceSize storageBuffer::getBufferSize() const
    {
        return m_count * m_size;
    }
