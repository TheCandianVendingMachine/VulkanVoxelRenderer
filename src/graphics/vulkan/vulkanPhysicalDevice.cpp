#include "graphics/vulkan/vulkanPhysicalDevice.hpp"
#include "graphics/vulkan/vulkanQueueFamilyIndices.hpp"
#include "graphics/vulkan/vulkanSwapChainSupportDetails.hpp"
#include "graphics/vulkan/vulkanHelpers.hpp"
#include "graphics/vulkan/vulkanConsts.hpp"
#include <vector>
#include <set>
#include <string>
#include <algorithm>

bool vulkanPhysicalDevice::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        vulkanQueueFamilyIndices indices = helpers::findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
            {
                vulkanSwapChainSupportDetails swapChainSupport = helpers::querySwapChainSupport(device, surface);
                swapChainAdequate = !swapChainSupport.m_formats.empty() && !swapChainSupport.m_presentModes.empty();
            }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

bool vulkanPhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> avaliableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, avaliableExtensions.data());

        std::set<std::string> requiredExtensions(std::begin(consts::c_deviceExtensions), std::end(consts::c_deviceExtensions));
        for (const auto &extension : avaliableExtensions)
            {
                requiredExtensions.erase(extension.extensionName);
            }

        return requiredExtensions.empty();
    }

vulkanPhysicalDevice::vulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
    {
        create(instance, surface);
    }

vulkanPhysicalDevice::~vulkanPhysicalDevice()
    {
        cleanup();
    }

void vulkanPhysicalDevice::create(VkInstance instance, VkSurfaceKHR surface)
    {
        vkEnumeratePhysicalDevices(instance, &m_deviceCount, nullptr);
        if (m_deviceCount == 0)
            {
                // <error>
                return;
            }

        std::vector<VkPhysicalDevice> devices(m_deviceCount);
        vkEnumeratePhysicalDevices(instance, &m_deviceCount, devices.data());

        for (const auto &device : devices)
            {
                if (isDeviceSuitable(device, surface))
                    {
                        m_physicalDevice = device;

                        m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

                        VkPhysicalDeviceProperties physicalDeviceProperties;
                        vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

                        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
                        if (counts & VK_SAMPLE_COUNT_64_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_64_BIT; }
                        if (counts & VK_SAMPLE_COUNT_32_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_32_BIT; }
                        if (counts & VK_SAMPLE_COUNT_16_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_16_BIT; }
                        if (counts & VK_SAMPLE_COUNT_8_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_8_BIT; }
                        if (counts & VK_SAMPLE_COUNT_4_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_4_BIT; }
                        if (counts & VK_SAMPLE_COUNT_2_BIT) { m_msaaSamples = VK_SAMPLE_COUNT_2_BIT; }

                        break;
                    }
            }

        if (m_physicalDevice == VK_NULL_HANDLE)
            {
                // <error>
                return;
            }

        m_cleanedUp = false;
    }

void vulkanPhysicalDevice::cleanup()
    {
        if (m_cleanedUp) { return; }

        m_physicalDevice = VK_NULL_HANDLE;
        m_cleanedUp = true;
    }

VkSampleCountFlagBits vulkanPhysicalDevice::getSampleCount() const
    {
        return m_msaaSamples;
    }

VkPhysicalDevice vulkanPhysicalDevice::getUnderlyingPhysicalDevice() const
    {
        return m_physicalDevice;
    }

vulkanPhysicalDevice::operator VkPhysicalDevice() const
    {
        return m_physicalDevice;
    }

VkPhysicalDevice &vulkanPhysicalDevice::getUnderlyingPhysicalDevice()
    {
        return m_physicalDevice;
    }
