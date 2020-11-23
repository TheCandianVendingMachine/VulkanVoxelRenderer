// vulkanRenderPass.hpp
// A wrapper around a VkRenderPass Vulkan struct
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "graphics/vulkan/vulkanSubpass.hpp"
#include "graphics/vulkan/vulkanAttachmentList.hpp"

class vulkanRenderPass
    {
        private:
            VkRenderPass m_renderPass = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanRenderPass() = default;
            vulkanRenderPass(VkDevice device, const std::vector<vulkanSubpass> &subpasses, const vulkanAttachmentList &attachments);
            ~vulkanRenderPass();
            void create(VkDevice device, const std::vector<vulkanSubpass> &subpasses, const vulkanAttachmentList &attachments);
            void cleanup();

            bool isCreated() const;

            VkRenderPass getUnderlyingRenderPass() const;
            operator VkRenderPass() const;

            VkRenderPass &getUnderlyingRenderPass();

    };
