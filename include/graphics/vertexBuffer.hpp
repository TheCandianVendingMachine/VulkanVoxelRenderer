// vertexBuffer.hpp
// A buffer that contains vertex information
#pragma once
#include <vector>
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/vertex.hpp"

class vulkanCommandBuffer;
class vertexBuffer
    {
        private:
            vulkanBuffer m_vertexBuffer;
            vulkanBuffer m_stagingVertexBuffer;
            std::vector<vertex> m_vertices;

            unsigned int m_size = 0;
            unsigned int m_bufferSize = 0;

            bool m_dynamic = false;
            bool m_updated = true;

        public:
            vertexBuffer() = default;
            vertexBuffer(unsigned int size, bool dynamic);
            ~vertexBuffer();
            void create(unsigned int size, bool dynamic);
            void destroy();
            void clear();
            
            void addVertex(vertex vertex);
            void addVertices(vertex *vertices, std::size_t count);
            void modifyVertex(unsigned int index, vertex newVertex);

            void updateBuffer(vulkanCommandBuffer &commandBuffer);
            bool needsUpdate() const;

            vulkanBuffer &getVertexBuffer();
            const vulkanBuffer &getVertexBuffer() const;

            unsigned int getVertexCount() const;
    };
