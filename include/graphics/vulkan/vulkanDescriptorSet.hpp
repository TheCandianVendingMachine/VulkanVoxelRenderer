// vulkanDescriptorSet.hpp
// A wrapper around VkDescriptorSet
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class vulkanDescriptorSet;
namespace vulkanDescriptorSetFunctions
    {
        void createBatch(std::vector<vulkanDescriptorSet> &descriptorSets, VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts);
    };

class vulkanDescriptorSet
    {
        private:
            VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
            VkDevice m_device = VK_NULL_HANDLE;

            bool m_cleanedUp = false;

            friend void vulkanDescriptorSetFunctions::createBatch(std::vector<vulkanDescriptorSet>&, VkDevice, VkDescriptorPool, const std::vector<VkDescriptorSetLayout>&);

        public:
            vulkanDescriptorSet() = default;
            vulkanDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts);
            ~vulkanDescriptorSet();
            void create(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout> &descriptorSetLayouts);
            void cleanup();

            VkDescriptorSet getUnderlyingDescriptorSet() const;
            operator VkDescriptorSet() const;

            VkDescriptorSet &getUnderlyingDescriptorSet();
    };