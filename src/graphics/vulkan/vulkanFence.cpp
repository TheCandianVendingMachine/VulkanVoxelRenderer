#include "graphics/vulkan/vulkanFence.hpp"

vulkanFence::vulkanFence(VkDevice device, VkFenceCreateFlags fenceFlags)
    {
        create(device, fenceFlags);
    }

vulkanFence::~vulkanFence()
    {
        cleanup();
    }

void vulkanFence::create(VkDevice device, VkFenceCreateFlags fenceFlags)
    {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = fenceFlags;

        if (vkCreateFence(device, &fenceInfo, nullptr, &m_fence) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanFence::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyFence(m_device, m_fence, nullptr);
        m_fence = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

bool vulkanFence::isCreated() const
    {
        return !m_cleanedUp;
    }

void vulkanFence::wait()
    {
        vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
    }

VkFence vulkanFence::getUnderlyingFence() const
    {
        return m_fence;
    }

vulkanFence::operator VkFence() const
    {
        return m_fence;
    }

VkFence &vulkanFence::getUnderlyingFence()
    {
        return m_fence;
    }
