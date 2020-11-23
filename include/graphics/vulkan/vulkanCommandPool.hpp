// vulkanCommandPool.hpp
// A wrapper around VkCommandPool
#pragma once
#include <vulkan/vulkan.h>
#include "graphics/vulkan/vulkanQueueFamilyIndices.hpp"

class vulkanCommandPool
    {
        private:
            VkCommandPool m_commandPool = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;
        public:
            vulkanCommandPool() = default;
            vulkanCommandPool(VkDevice device, vulkanQueueFamilyIndices queueFamilyIndices);
            ~vulkanCommandPool();
            void create(VkDevice device, vulkanQueueFamilyIndices queueFamilyIndices);
            void cleanup();

            bool isCreated() const;

            VkCommandPool getUnderlyingCommandPool() const;
            operator VkCommandPool() const;

            VkCommandPool &getUnderlyingCommandPool();

    };
