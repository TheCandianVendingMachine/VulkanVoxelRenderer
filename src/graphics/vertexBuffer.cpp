#include "graphics/vertexBuffer.hpp"
#include "graphics/vulkan/vulkanAllocator.hpp"
#include "graphics/vulkan/vulkanCommandBuffer.hpp"

vertexBuffer::vertexBuffer(unsigned int size, bool dynamic)
    {
        create(size, dynamic);
    }

vertexBuffer::~vertexBuffer()
    {
        destroy();
    }

void vertexBuffer::create(unsigned int size, bool dynamic)
    {
        m_size = size;
        m_bufferSize = size * sizeof(vertex);
        m_stagingVertexBuffer.create(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_COPY);

        VmaMemoryUsage bufferUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (dynamic)
            {
                bufferUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            }
        m_vertexBuffer.create(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferUsage);
    }

void vertexBuffer::destroy()
    {
        m_vertexBuffer.cleanup();
        m_stagingVertexBuffer.cleanup();
        m_vertices.clear();
        m_size = 0;
        m_bufferSize = 0;
    }

void vertexBuffer::addVertex(vertex vertex)
    {
        m_vertices.push_back(vertex);
        m_updated = true;
    }

void vertexBuffer::addVertices(vertex *vertices, std::size_t count)
    {
        std::size_t initialIndex = m_vertices.size();
        m_vertices.resize(m_vertices.size() + count);
        for (std::size_t i = 0; i < count; i++)
            {
                m_vertices.at(initialIndex + i) = vertices[i];
            }
        m_updated = true;
    }

void vertexBuffer::modifyVertex(unsigned int index, vertex newVertex)
    {
        m_vertices[index] = newVertex;
        m_updated = true;
    }

void vertexBuffer::updateBuffer(vulkanCommandBuffer &commandBuffer)
    {
        if (m_vertices.empty()) { m_updated = false; return; }
        if (m_vertices.size() > m_size)
            {
                m_vertexBuffer.cleanup();
                m_stagingVertexBuffer.cleanup();
                create(m_vertices.size(), m_dynamic);
            }

        void *data = nullptr;
        vmaMapMemory(*globals::g_vulkanAllocator, m_stagingVertexBuffer.getUnderlyingAllocation(), &data);
        std::memcpy(data, m_vertices.data(), sizeof(std::vector<vertex>::value_type) * m_vertices.size());
        vmaUnmapMemory(*globals::g_vulkanAllocator, m_stagingVertexBuffer.getUnderlyingAllocation());

        VkBufferCopy copyRegion{};
        copyRegion.size = m_bufferSize;
        vkCmdCopyBuffer(commandBuffer, m_stagingVertexBuffer, m_vertexBuffer, 1, &copyRegion);

        m_updated = false;
    }

bool vertexBuffer::needsUpdate() const
    {
        return m_updated;
    }

vulkanBuffer &vertexBuffer::getVertexBuffer()
    {
        return m_vertexBuffer;
    }

const vulkanBuffer &vertexBuffer::getVertexBuffer() const
    {
        return m_vertexBuffer;
    }

unsigned int vertexBuffer::getVertexCount() const
    {
        return static_cast<unsigned int>(m_vertices.size());
    }
