// vulkanPipelineLayout.hpp
// Wrapper for pipeline layout
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class vulkanPipelineLayout
    {
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanPipelineLayout() = default;
            vulkanPipelineLayout(VkDevice device, const std::vector<VkDescriptorSetLayout> &layoutInfo);
            ~vulkanPipelineLayout();
            void create(VkDevice device, const std::vector<VkDescriptorSetLayout> &layoutInfo);
            void cleanup();

            bool isCreated() const;

            VkPipelineLayout getUnderlyingPipelineLayout() const;
            operator VkPipelineLayout() const;

            VkPipelineLayout &getUnderlyingPipelineLayout();

    };
