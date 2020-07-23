#include "graphics/vulkan/vulkanAllocator.hpp"

vulkanAllocator *globals::g_vulkanAllocator = nullptr;

vulkanAllocator::vulkanAllocator(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice)
    {
        create(instance, device, physicalDevice);
    }

vulkanAllocator::~vulkanAllocator()
    {
        cleanup();
    }

void vulkanAllocator::create(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice)
    {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.instance = instance;
        createInfo.device = device;
        createInfo.physicalDevice = physicalDevice;

        vmaCreateAllocator(&createInfo, &m_allocator);
        m_cleanedUp = false;
    }

void vulkanAllocator::cleanup()
    {
        if (m_cleanedUp) { return; }

        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VmaAllocator vulkanAllocator::getUnderlyingAllocator() const
    {
        return m_allocator;
    }

VmaAllocator &vulkanAllocator::getUnderlyingAllocator()
    {
        return m_allocator;
    }

vulkanAllocator::operator VmaAllocator() const
    {
        return m_allocator;
    }
