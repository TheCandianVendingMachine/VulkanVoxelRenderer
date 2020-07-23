#include "graphics/renderPass.hpp"
#include "graphics/vulkan/vulkanPhysicalDevice.hpp"
#include "graphics/vulkan/vulkanHelpers.hpp"
#include "graphics/vulkan/vulkanSwapChain.hpp"
#include <array>

VkFormat renderPass::findDepthFormat(VkPhysicalDevice physicalDevice) const
    {
        return helpers::findSupportedFormat(
            physicalDevice,
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

void renderPass::create(VkDevice device, vulkanPhysicalDevice &physicalDevice, const vulkanSwapChain &swapChain, std::function<void(std::vector<vulkanSubpass>&, vulkanAttachmentList&)> renderPassInit)
    {
        std::vector<vulkanSubpass> subpasses;
        vulkanAttachmentList attachmentList;
        renderPassInit(subpasses, attachmentList);
        subpasses.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0);
        VkAttachmentDescription colourAttachment{};
        colourAttachment.format = swapChain.getFormat();
        colourAttachment.samples = physicalDevice.getSampleCount();
        colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat(physicalDevice);
        depthAttachment.samples = physicalDevice.getSampleCount();
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colourAttachmentResolve{};
        colourAttachmentResolve.format = swapChain.getFormat();
        colourAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colourAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colourAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colourAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colourAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colourAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colourAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachmentList.addAttachment(vulkanAttachmentDescription(colourAttachment, vulkanAttachmentDescription::attachmentType::COLOUR_ATTACHMENT), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        attachmentList.addAttachment(vulkanAttachmentDescription(depthAttachment, vulkanAttachmentDescription::attachmentType::STENCIL_ATTACHMENT), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        attachmentList.addAttachment(vulkanAttachmentDescription(colourAttachmentResolve, vulkanAttachmentDescription::attachmentType::RESOLVE_ATTACHMENT), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        m_renderPass.create(device, subpasses, attachmentList);

        m_colourImage.create(device, VK_IMAGE_ASPECT_COLOR_BIT, swapChain.getExtent().width, swapChain.getExtent().height, 1, physicalDevice.getSampleCount(), swapChain.getFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        m_depthImage.create(device, VK_IMAGE_ASPECT_DEPTH_BIT, swapChain.getExtent().width, swapChain.getExtent().height, 1, physicalDevice.getSampleCount(), findDepthFormat(physicalDevice), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        m_frameBuffers.resize(swapChain.getImageViews().size());
        for (std::size_t i = 0; i < m_frameBuffers.size(); i++)
            {
                std::array<VkImageView, 3> attachments = {
                    m_colourImage.getImageView(),
                    m_depthImage.getImageView(),
                    swapChain.getImageViews()[i],
                };

                m_frameBuffers[i].create(device, m_renderPass, static_cast<unsigned int>(attachments.size()), attachments.data(), swapChain.getExtent().width, swapChain.getExtent().height);
            }

    }

void renderPass::cleanup()
    {
        for (auto &framebuffer : m_frameBuffers)
            {
                framebuffer.cleanup();
            }
        m_depthImage.cleanup();
        m_colourImage.cleanup();
        m_renderPass.cleanup();
    }

vulkanRenderPass &renderPass::getRenderPass()
    {
        return m_renderPass;
    }

std::vector<vulkanFramebuffer> &renderPass::getFrameBuffers()
    {
        return m_frameBuffers;
    }
