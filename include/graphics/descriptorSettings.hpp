// descriptorSettings.hpp
// A setting module to plug into the descriptor set and handler to determine shader descriptors
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class descriptorSettings
    {
        private:
            struct descriptor
                {
                    VkDescriptorType m_type = VK_DESCRIPTOR_TYPE_BEGIN_RANGE;
                    VkShaderStageFlags m_shaderStages = VK_SHADER_STAGE_ALL;
                    unsigned int m_descriptorCount = 0;
                    unsigned int m_bindingNumber = -1;
                };

            std::vector<descriptor> m_descriptorSettings;

        public:
            void addSetting(VkDescriptorType type, VkShaderStageFlags stages, unsigned int count = 1);

            std::vector<VkDescriptorSetLayoutBinding> getLayoutBindings() const;
            std::vector<VkDescriptorPoolSize> getPoolSizes(unsigned int swapChainImageCount) const;
            std::vector<VkWriteDescriptorSet> getDescriptorWrites() const;

    };
