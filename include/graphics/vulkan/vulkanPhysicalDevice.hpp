// vulkanPhysicalDevice.hpp
// Defines a wrapper around VkPhysicalDevice
#pragma once
#include <vulkan/vulkan.h>

class vulkanPhysicalDevice
    {
        private:
            VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
            VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

            uint32_t m_deviceCount = 0;

            bool m_cleanedUp = true;

            bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        public:
            vulkanPhysicalDevice() = default;
            vulkanPhysicalDevice(VkInstance instace, VkSurfaceKHR surface);
            ~vulkanPhysicalDevice();
            void create(VkInstance instance, VkSurfaceKHR surface);
            void cleanup();

            VkSampleCountFlagBits getSampleCount() const;

            VkPhysicalDevice getUnderlyingPhysicalDevice() const;
            operator VkPhysicalDevice() const;

            VkPhysicalDevice &getUnderlyingPhysicalDevice();

    };
