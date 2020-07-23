// vulkanSubPass.hpp
// Wrapper for subpasse data. Handles dependencies for the subpass and has methods to generate constructor structs
#pragma once
#include <vulkan/vulkan.h>

class vulkanAttachmentList;
class vulkanSubpass
    {
        private:
            VkPipelineStageFlags m_sourceStageMask = 0;
            VkPipelineStageFlags m_destinationStageMask = 0;
            VkAccessFlags m_sourceAccessMask = 0;
            VkAccessFlags m_destinationAccessMask = 0;
            VkDependencyFlags m_dependencyFlags = 0;

        public:
            vulkanSubpass() = default;
            vulkanSubpass(VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags destinationStageMask, VkAccessFlags sourceAccessMask, VkAccessFlags destinationAccessMask, VkDependencyFlags dependencyFlags);
            
            void setSourceStageMask(VkPipelineStageFlags sourceStageMask);
            void setDestinationStageMask(VkPipelineStageFlags destinationStageMask);
            void setSourceAccessMask(VkAccessFlags sourceAccessMask);
            void setDestinationAccessMask(VkAccessFlags destinationAccessMask);
            void setDependencyFlags(VkDependencyFlags dependencyFlags);

            VkSubpassDescription getSubpassDescription(const vulkanAttachmentList &attachments) const;
            VkSubpassDependency getSubpassDependency() const;
            
    };
