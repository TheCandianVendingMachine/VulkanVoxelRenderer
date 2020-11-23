// vulkanDescriptorSetLayout.hpp
// Wrapper around vkDescriptorSetLayout
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class vulkanDescriptorSetLayout
    {
        private:
            VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanDescriptorSetLayout() = default;
            vulkanDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &layoutBindings);
            ~vulkanDescriptorSetLayout();
            void create(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &layoutBindings);
            void cleanup();

            bool isCreated() const;

            VkDescriptorSetLayout getUnderlyingDescriptorSetLayout() const;
            operator VkDescriptorSetLayout() const;

            VkDescriptorSetLayout &getUnderlyingDescriptorSetLayout();

    };
