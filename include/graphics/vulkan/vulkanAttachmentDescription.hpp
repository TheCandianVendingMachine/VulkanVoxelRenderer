// vulkanAttachmentDescription.hpp
// Wrapper around a VkAttachmentDescription with an enum to determine attachment type
#pragma once
#include <vulkan/vulkan.h>

struct vulkanAttachmentDescription
    {
        VkAttachmentDescription m_attachment{};
        enum class attachmentType
            {
                INPUT_ATTACHMENT,
                COLOUR_ATTACHMENT,
                RESOLVE_ATTACHMENT,
                STENCIL_ATTACHMENT,
            } m_attachmentType;
        vulkanAttachmentDescription(VkAttachmentDescription description, attachmentType type) :
            m_attachment(description),
            m_attachmentType(type)
        {
        }
    };
