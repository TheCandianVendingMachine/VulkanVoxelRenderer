// vulkanAttachmentList.hpp
// Defines a list of attachments for a renderpass to use in its creation
#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include "graphics/vulkan/vulkanAttachmentDescription.hpp"

class vulkanAttachmentList
    {
        private:
            std::unordered_map<vulkanAttachmentDescription::attachmentType, std::vector<VkAttachmentReference>> m_attachmentDescriptions;
            std::vector<VkAttachmentDescription> m_attachmentDependencies;
            uint32_t m_attachmentCount = 0;

        public:
            void addAttachment(vulkanAttachmentDescription attachment, VkImageLayout layout);
           
            const std::vector<VkAttachmentDescription> &getSubpassAttachments() const;
            const std::vector<VkAttachmentReference> &getAttachmentTypeReferences(vulkanAttachmentDescription::attachmentType type) const;
            uint32_t getAttachmentTypeCount(vulkanAttachmentDescription::attachmentType type) const;
            uint32_t getAttachmentCount() const;

    };
