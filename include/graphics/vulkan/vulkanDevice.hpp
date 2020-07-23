// vulkanDevice.hpp
// Defines a wrapper around VkDevice
#pragma once
#include <vulkan/vulkan.h>

class vulkanDevice
    {
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            bool m_cleanedUp = false;

        public:
            vulkanDevice() = default;
            vulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
            ~vulkanDevice();
            void create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
            void cleanup();

            VkDevice getUnderlyingDevice() const;
            operator VkDevice() const;

            VkDevice &getUnderlyingDevice();

    };
