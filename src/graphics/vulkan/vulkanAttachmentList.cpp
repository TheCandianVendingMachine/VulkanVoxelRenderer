#include "graphics/vulkan/vulkanAttachmentList.hpp"

void vulkanAttachmentList::addAttachment(vulkanAttachmentDescription attachment, VkImageLayout layout)
    {
        VkAttachmentReference reference{};
        reference.attachment = m_attachmentCount++;
        reference.layout = layout;

        m_attachmentDescriptions[attachment.m_attachmentType].push_back(reference);
        m_attachmentDependencies.push_back(attachment.m_attachment);
    }

const std::vector<VkAttachmentDescription> &vulkanAttachmentList::getSubpassAttachments() const
    {
        return m_attachmentDependencies;
    }

const std::vector<VkAttachmentReference> &vulkanAttachmentList::getAttachmentTypeReferences(vulkanAttachmentDescription::attachmentType type) const
    {
        if (getAttachmentTypeCount(type) > 0)
            {
                return m_attachmentDescriptions.at(type);
            }
        return {};
    }

uint32_t vulkanAttachmentList::getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType type) const
    {
        return static_cast<uint32_t>(m_attachmentDescriptions.count(type));
    }

uint32_t vulkanAttachmentList::getAttachmentCount() const
    {
        return m_attachmentCount;
    }
