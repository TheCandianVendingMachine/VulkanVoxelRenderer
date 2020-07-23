#include "graphics/vulkan/vulkanDevice.hpp"
#include "graphics/vulkan/vulkanQueueFamilyIndices.hpp"
#include "graphics/vulkan/vulkanHelpers.hpp"
#include "graphics/vulkan/vulkanConsts.hpp"
#include <vector>
#include <set>

vulkanDevice::vulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        create(physicalDevice, surface);
    }

vulkanDevice::~vulkanDevice()
    {
        cleanup();
    }

void vulkanDevice::create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        vulkanQueueFamilyIndices indices = helpers::findQueueFamilies(physicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.m_graphicsFamily.value(), indices.m_presentFamily.value() };

        float queuePriority = 1.f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.m_graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = consts::c_deviceExtensionSize;
        createInfo.ppEnabledExtensionNames = consts::c_deviceExtensions;

        if (consts::c_enableValidationLayers)
            {
                createInfo.enabledLayerCount = consts::c_validationLayersSize;
                createInfo.ppEnabledLayerNames = consts::c_validationLayers;
            }
        else
            {
                createInfo.enabledLayerCount = 0;
            }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_cleanedUp = false;
    }

void vulkanDevice::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;

        m_cleanedUp = false;
    }

VkDevice vulkanDevice::getUnderlyingDevice() const
    {
        return m_device;
    }

vulkanDevice::operator VkDevice() const
    {
        return m_device;
    }

VkDevice &vulkanDevice::getUnderlyingDevice()
    {
        return m_device;
    }
