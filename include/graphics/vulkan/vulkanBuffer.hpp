// vulkanBuffer.hpp
// A wrapper around VkBuffer. Also handles its own memory
#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class vulkanBuffer
    {
        private:
            VkBuffer m_buffer = VK_NULL_HANDLE;
            VmaAllocation m_allocation = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanBuffer() = default;
            vulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaMemoryUsage memoryUsage);
            ~vulkanBuffer();
            void create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaMemoryUsage memoryUsage);
            void cleanup();

            bool isCreated() const;

            VkBuffer getUnderlyingBuffer() const;
            operator VkBuffer() const;

            VmaAllocation getUnderlyingAllocation() const;
            operator VmaAllocation() const;

            VkBuffer &getUnderlyingBuffer();
            VmaAllocation &getUnderlyingAllocation();

    };
