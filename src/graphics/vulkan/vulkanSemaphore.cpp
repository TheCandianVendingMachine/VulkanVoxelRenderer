#include "graphics/vulkan/vulkanSemaphore.hpp"

vulkanSemaphore::vulkanSemaphore(VkDevice device)
    {
        create(device);
    }

vulkanSemaphore::~vulkanSemaphore()
    {
        cleanup();
    }

void vulkanSemaphore::create(VkDevice device)
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_semaphore) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanSemaphore::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroySemaphore(m_device, m_semaphore, nullptr);
        m_semaphore = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VkSemaphore vulkanSemaphore::getUnderlyingSemaphore() const
    {
        return m_semaphore;
    }

vulkanSemaphore::operator VkSemaphore() const
    {
        return m_semaphore;
    }

VkSemaphore &vulkanSemaphore::getUnderlyingSemaphore()
    {
        return m_semaphore;
    }
