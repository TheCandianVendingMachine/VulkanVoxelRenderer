// vulkanSemaphore.hpp
// A wrapper around VkSemaphore
#pragma once
#include <vulkan/vulkan.h>

class vulkanSemaphore
    {
        private:
            VkSemaphore m_semaphore = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanSemaphore() = default;
            vulkanSemaphore(VkDevice device);
            ~vulkanSemaphore();
            void create(VkDevice device);
            void cleanup();

            bool isCreated() const;

            VkSemaphore getUnderlyingSemaphore() const;
            operator VkSemaphore() const;

            VkSemaphore &getUnderlyingSemaphore();
    };