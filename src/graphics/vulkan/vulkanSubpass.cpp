#include "graphics/vulkan/vulkanSubpass.hpp"
#include "graphics/vulkan/vulkanAttachmentDescription.hpp"
#include "graphics/vulkan/vulkanAttachmentList.hpp"

vulkanSubpass::vulkanSubpass(VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags destinationStageMask, VkAccessFlags sourceAccessMask, VkAccessFlags destinationAccessMask, VkDependencyFlags dependencyFlags)
    {
        setSourceStageMask(sourceStageMask);
        setDestinationStageMask(destinationStageMask);
        setSourceAccessMask(sourceAccessMask);
        setDestinationAccessMask(destinationAccessMask);
        setDependencyFlags(dependencyFlags);
    }

void vulkanSubpass::setSourceStageMask(VkPipelineStageFlags sourceStageMask)
    {
        m_sourceStageMask = sourceStageMask;
    }

void vulkanSubpass::setDestinationStageMask(VkPipelineStageFlags destinationStageMask)
    {
        m_destinationStageMask = destinationStageMask;
    }

void vulkanSubpass::setSourceAccessMask(VkAccessFlags sourceAccessMask)
    {
        m_sourceAccessMask = sourceAccessMask;
    }

void vulkanSubpass::setDestinationAccessMask(VkAccessFlags destinationAccessMask)
    {
        m_destinationAccessMask = destinationAccessMask;
    }

void vulkanSubpass::setDependencyFlags(VkDependencyFlags dependencyFlags)
    {
        m_dependencyFlags = dependencyFlags;
    }

VkSubpassDescription vulkanSubpass::getSubpassDescription(const vulkanAttachmentList &attachments) const
    {
        VkSubpassDescription description{};
        description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        description.inputAttachmentCount = 0;
        description.colorAttachmentCount = 0;

        description.pInputAttachments = nullptr;
        if (attachments.getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType::INPUT_ATTACHMENT) > 0)
            {
                description.inputAttachmentCount = attachments.getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType::INPUT_ATTACHMENT);
                description.pInputAttachments = attachments.getAttachmentTypeReferences(vulkanAttachmentDescription::attachmentType::INPUT_ATTACHMENT).data();
            }

        description.pColorAttachments = nullptr;
        if (attachments.getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType::COLOUR_ATTACHMENT) > 0)
            {
                description.colorAttachmentCount = attachments.getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType::COLOUR_ATTACHMENT);
                description.pColorAttachments = attachments.getAttachmentTypeReferences(vulkanAttachmentDescription::attachmentType::COLOUR_ATTACHMENT).data();
            }

        description.pResolveAttachments = nullptr;
        if (attachments.getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType::RESOLVE_ATTACHMENT) > 0)
            {
                description.pResolveAttachments = attachments.getAttachmentTypeReferences(vulkanAttachmentDescription::attachmentType::RESOLVE_ATTACHMENT).data();
            }

        description.pDepthStencilAttachment = nullptr;
        if (attachments.getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType::STENCIL_ATTACHMENT) > 0)
            {
                description.pDepthStencilAttachment = attachments.getAttachmentTypeReferences(vulkanAttachmentDescription::attachmentType::STENCIL_ATTACHMENT).data();
            }

        return description;
    }

VkSubpassDependency vulkanSubpass::getSubpassDependency() const
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = m_sourceStageMask;
        dependency.srcAccessMask = m_sourceAccessMask;
        dependency.dstStageMask = m_destinationStageMask;
        dependency.dstAccessMask = m_destinationAccessMask;
        dependency.dependencyFlags = m_dependencyFlags;
        return dependency;
    }
