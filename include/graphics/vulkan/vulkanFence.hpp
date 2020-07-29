// vulkanFence.hpp
// A wrapper around VkFence
#pragma once
#include <vulkan/vulkan.h>

class vulkanFence
    {
        private:
            VkFence m_fence = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanFence() = default;
            vulkanFence(VkDevice device, VkFenceCreateFlags fenceFlags);
            ~vulkanFence();
            void create(VkDevice device, VkFenceCreateFlags fenceFlags);
            void cleanup();

            void wait();

            VkFence getUnderlyingFence() const;
            operator VkFence() const;

            VkFence &getUnderlyingFence();

    };