#include "graphics/uniformBuffer.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"
#include <cstring>

uniformBuffer::uniformBuffer(unsigned int size)
    {
        create(size);
    }

uniformBuffer::~uniformBuffer()
    {
        destroy();
    }

void uniformBuffer::create(unsigned int size)
    {
        m_uniformBuffer.create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        m_size = size;
    }

void uniformBuffer::destroy()
    {
        m_uniformBuffer.cleanup();
    }

void uniformBuffer::bind(void *data)
    {
        void *tempData = nullptr;
        vmaMapMemory(*globals::g_vulkanAllocator, m_uniformBuffer, &tempData);
        std::memcpy(tempData, data, m_size);
        vmaUnmapMemory(*globals::g_vulkanAllocator, m_uniformBuffer);
    }

vulkanBuffer &uniformBuffer::getUniformBuffer()
    {
        return m_uniformBuffer;
    }

const vulkanBuffer &uniformBuffer::getUniformBuffer() const
    {
        return m_uniformBuffer;
    }

unsigned int uniformBuffer::getBufferSize() const
    {
        return m_size;
    }
