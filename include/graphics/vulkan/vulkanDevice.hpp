// vulkanDevice.hpp
// Defines a wrapper around VkDevice
#pragma once
#include <vulkan/vulkan.h>

class vulkanDevice
    {
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            bool m_cleanedUp = true;

        public:
            vulkanDevice() = default;
            vulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
            ~vulkanDevice();
            void create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
            void cleanup();

            bool isCreated() const;

            VkDevice getUnderlyingDevice() const;
            operator VkDevice() const;

            VkDevice &getUnderlyingDevice();

    };
