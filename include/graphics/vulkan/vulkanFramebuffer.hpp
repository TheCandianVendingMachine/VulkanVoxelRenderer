// vulkanFramebuffer.hpp
// Wrapper around VkFramebuffer
#pragma once
#include <vulkan/vulkan.h>

class vulkanFramebuffer
    {
        private:
            VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = false;

        public:
            vulkanFramebuffer() = default;
            vulkanFramebuffer(VkDevice device, VkRenderPass renderPass, unsigned int attachmentCount, VkImageView *attachments, unsigned int width, unsigned int height);
            ~vulkanFramebuffer();
            void create(VkDevice device, VkRenderPass renderPass, unsigned int attachmentCount, VkImageView *attachments, unsigned int width, unsigned int height);
            void cleanup();

            VkFramebuffer getUnderlyingFrameBuffer() const;
            operator VkFramebuffer() const;

            VkFramebuffer &getUnderlyingFrameBuffer();

    };
