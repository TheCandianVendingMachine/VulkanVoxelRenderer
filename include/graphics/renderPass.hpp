// renderPass.hpp
// A single pass on a render. Owns its own frame buffer, attachments, etc
#pragma once
#include <vector>
#include <functional>
#include <vulkan/vulkan.h>
#include "graphics/vulkan/vulkanRenderPass.hpp"
#include "graphics/vulkan/vulkanFramebuffer.hpp"
#include "graphics/image.hpp"

class vulkanSwapChain;
class vulkanPhysicalDevice;
class renderPass
    {
        private:
            vulkanRenderPass m_renderPass;
            std::vector<vulkanFramebuffer> m_frameBuffers;

            image m_colourImage;
            image m_depthImage;

            VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) const;

        public:
            void create(VkDevice device, vulkanPhysicalDevice &physicalDevice, const vulkanSwapChain &swapChain, std::function<void(std::vector<vulkanSubpass>&, vulkanAttachmentList&)> renderPassInit);
            void cleanup();
            vulkanRenderPass &getRenderPass();
            std::vector<vulkanFramebuffer> &getFrameBuffers();

    };
