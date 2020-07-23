#include "graphics/indexBuffer.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"
#include "graphics/vulkan/vulkanCommandBuffer.hpp"

indexBuffer::indexBuffer(unsigned int size, bool dynamic)
    {
        create(size, dynamic);
    }

indexBuffer::~indexBuffer()
    {
        destroy();
    }

void indexBuffer::create(unsigned int size, bool dynamic)
    {
        m_size = size;
        m_bufferSize = size * sizeof(fe::index);
        m_stagingIndexBuffer.create(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_COPY);

        VmaMemoryUsage bufferUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (dynamic)
            {
                bufferUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            }
        m_indexBuffer.create(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferUsage);
    }

void indexBuffer::destroy()
    {
        m_indexBuffer.cleanup();
        m_stagingIndexBuffer.cleanup();
    }

void indexBuffer::addIndex(fe::index index)
    {
        m_indices.push_back(index);
        m_updated = true;
    }

void indexBuffer::addIndices(fe::index *indices, std::size_t count)
    {
        std::size_t initialIndex = m_indices.size();
        m_indices.resize(m_indices.size() + count);
        for (std::size_t i = 0; i < count; i++)
            {
                m_indices.at(initialIndex + i) = indices[i];
            }
        m_updated = true;
    }

void indexBuffer::updateBuffer(vulkanCommandBuffer &commandBuffer)
    {
        if (m_indices.size() > m_size)
            {
                destroy();
                create(m_indices.size(), m_dynamic);
            }

        void *data = nullptr;
        vmaMapMemory(*globals::g_vulkanAllocator, m_stagingIndexBuffer.getUnderlyingAllocation(), &data);
        std::memcpy(data, m_indices.data(), sizeof(std::vector<fe::index>::value_type) * m_indices.size());
        vmaUnmapMemory(*globals::g_vulkanAllocator, m_stagingIndexBuffer.getUnderlyingAllocation());

        VkBufferCopy copyRegion{};
        copyRegion.size = m_bufferSize;
        vkCmdCopyBuffer(commandBuffer, m_stagingIndexBuffer, m_indexBuffer, 1, &copyRegion);

        m_updated = false;
    }

bool indexBuffer::needsUpdate() const
    {
        return m_updated;
    }

vulkanBuffer &indexBuffer::getIndexBuffer()
    {
        return m_indexBuffer;
    }

const vulkanBuffer &indexBuffer::getIndexBuffer() const
    {
        return m_indexBuffer;
    }

unsigned int indexBuffer::getIndexCount() const
    {
        return m_size;
    }
