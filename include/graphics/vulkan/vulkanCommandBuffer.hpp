// vulkanCommandBuffer.hpp
// A wrapper around VkCommandBuffer
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class vulkanCommandBuffer;
namespace vulkanCommandBufferFunctions
    {
        void createBatch(std::vector<vulkanCommandBuffer> &commandBuffers, VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level);
    }

class vulkanCommandBuffer
    {
        private:
            VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

            friend void vulkanCommandBufferFunctions::createBatch(std::vector<vulkanCommandBuffer>&, VkDevice, VkCommandPool, VkCommandBufferLevel);

        public:
            vulkanCommandBuffer() = default;
            vulkanCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level);
            ~vulkanCommandBuffer();
            void create(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level);
            void cleanup();

            VkCommandBuffer getUnderlyingCommandBuffer() const;
            operator VkCommandBuffer() const;

            VkCommandBuffer &getUnderlyingCommandBuffer();

    };
