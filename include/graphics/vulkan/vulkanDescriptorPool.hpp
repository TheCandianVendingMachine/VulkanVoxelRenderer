// vulkanDescriptorPool.hpp
// A wrapper around VkDescriptorPool
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class vulkanDescriptorPool
    {
        private:
            VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

        public:
            vulkanDescriptorPool() = default;
            vulkanDescriptorPool(VkDevice device, unsigned int descriptorSetCount, const std::vector<VkDescriptorPoolSize> &descriptors);
            ~vulkanDescriptorPool();
            void create(VkDevice device, unsigned int descriptorSetCount, const std::vector<VkDescriptorPoolSize> &descriptors);
            void cleanup();

            VkDescriptorPool getUnderlyingDescriptorPool() const;
            operator VkDescriptorPool() const;

            VkDescriptorPool &getUnderlyingDescriptorPool();

    };
