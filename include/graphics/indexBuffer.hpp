// indexBuffer.hpp
// A buffer for indices to share vertices
#pragma once
#include <vector>
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "typeDefines.hpp"

class vulkanCommandBuffer;
class indexBuffer
    {
        private:
            vulkanBuffer m_indexBuffer;
            vulkanBuffer m_stagingIndexBuffer;
            std::vector<fe::index> m_indices;

            unsigned int m_size = 0;
            unsigned int m_bufferSize = 0;

            bool m_dynamic = false;
            bool m_updated = true;

        public:
            indexBuffer() = default;
            indexBuffer(unsigned int size, bool dynamic);
            ~indexBuffer();
            void create(unsigned int size, bool dynamic);
            void destroy();
            void clear();
            
            void addIndex(fe::index index);
            void addIndices(fe::index *indices, std::size_t count);
            void modifyIndex(unsigned int position, fe::index newIndex);

            void updateBuffer(vulkanCommandBuffer &commandBuffer);
            bool needsUpdate() const;

            vulkanBuffer &getIndexBuffer();
            const vulkanBuffer &getIndexBuffer() const;

            unsigned int getIndexCount() const;
    };
