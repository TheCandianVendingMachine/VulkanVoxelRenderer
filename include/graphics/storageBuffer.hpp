// storageBuffer.hpp
// A dynamic buffer to transfer data to GPU shaders
#pragma once
#include "graphics/vulkan/vulkanBuffer.hpp"
#include <cstddef>

class vulkanDevice;
class vulkanCommandBuffer;
class storageBuffer
    {
        private:
            vulkanBuffer m_storageBuffer;
            unsigned int m_count = 0;
            std::size_t m_size = 0;

            bool m_dynamic = false;

        public:
            storageBuffer() = default;
            storageBuffer(unsigned int count, std::size_t objectSize);
            storageBuffer(unsigned int count, std::size_t objectSize, void *data);
            ~storageBuffer();

            void create(unsigned int count, std::size_t objectSize);
            void create(unsigned int count, std::size_t objectSize, void *data);

            void destroy();
            
            void bind(void *data);

            const vulkanBuffer &getStorageBuffer() const;
            vulkanBuffer &getStorageBuffer();

            unsigned int getBufferCount() const;
            std::size_t getBufferObjectSize() const;
            VkDeviceSize getBufferSize() const;

    };

