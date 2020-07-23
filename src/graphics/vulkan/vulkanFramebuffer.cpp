#include "graphics/vulkan/vulkanFramebuffer.hpp"

vulkanFramebuffer::vulkanFramebuffer(VkDevice device, VkRenderPass renderPass, unsigned int attachmentCount, VkImageView *attachments, unsigned int width, unsigned int height)
    {
        create(device, renderPass, attachmentCount, attachments, width, height);
    }

vulkanFramebuffer::~vulkanFramebuffer()
    {
        cleanup();
    }

void vulkanFramebuffer::create(VkDevice device, VkRenderPass renderPass, unsigned int attachmentCount, VkImageView *attachments, unsigned int width, unsigned int height)
    {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentCount);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = static_cast<uint32_t>(width);
        framebufferInfo.height = static_cast<uint32_t>(height);
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanFramebuffer::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
        m_framebuffer = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VkFramebuffer vulkanFramebuffer::getUnderlyingFrameBuffer() const
    {
        return m_framebuffer;
    }

vulkanFramebuffer::operator VkFramebuffer() const
    {
        return m_framebuffer;
    }

VkFramebuffer &vulkanFramebuffer::getUnderlyingFrameBuffer()
    {
        return m_framebuffer;
    }
