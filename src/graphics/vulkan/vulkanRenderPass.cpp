#include "graphics/vulkan/vulkanRenderPass.hpp"
#include <cstring>

vulkanRenderPass::vulkanRenderPass(VkDevice device, const std::vector<vulkanSubpass> &subpasses, const vulkanAttachmentList &attachments)
    {
        create(device, subpasses, attachments);
    }

vulkanRenderPass::~vulkanRenderPass()
    {
        cleanup();
    }

void vulkanRenderPass::create(VkDevice device, const std::vector<vulkanSubpass> &subpasses, const vulkanAttachmentList &attachments)
    {
        // pre-processing of subpasses 
        std::vector<VkSubpassDescription> subpassDescriptions;
        std::vector<VkSubpassDependency> subpassDependencies;
        for (const auto &subpass : subpasses)
            {
                subpassDescriptions.push_back(subpass.getSubpassDescription(attachments));

                VkSubpassDependency dependency = subpass.getSubpassDependency();
                if (std::find_if(subpassDependencies.begin(), subpassDependencies.end(), [&dependency](const VkSubpassDependency &testDependency){
                    return std::memcmp(&dependency, &testDependency, sizeof(dependency)) == 0;
                }) == subpassDependencies.end())
                    {
                        subpassDependencies.push_back(dependency);
                    }
            }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachments.getAttachmentCount();
        renderPassInfo.pAttachments = attachments.getSubpassAttachments().data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
        renderPassInfo.pSubpasses = subpassDescriptions.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanRenderPass::cleanup()
    {
        if (m_cleanedUp) { return; }
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
        m_cleanedUp = true;
    }

VkRenderPass vulkanRenderPass::getUnderlyingRenderPass() const
    {
        return m_renderPass;
    }

vulkanRenderPass::operator VkRenderPass() const
    {
        return m_renderPass;
    }

VkRenderPass &vulkanRenderPass::getUnderlyingRenderPass()
    {
        return m_renderPass;
    }
