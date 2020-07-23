// descriptorSet.hpp
// A manager object for vulkanDescriptorSet. It handles its own creation
#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include "graphics/vulkan/vulkanDescriptorSet.hpp"

class vulkanDevice;
class vulkanDescriptorPool;
class vulkanImageView;
class vulkanSampler;
class vulkanBuffer;
class descriptorSet
    {
        private:
            std::vector<vulkanDescriptorSet> m_descriptorSets;
            std::vector<VkDescriptorBufferInfo> m_bufferInfo;
            VkDescriptorImageInfo m_imageInfo;
            bool m_needsUpdate = false;

            vulkanDevice *m_device = nullptr;

        public:
            void create(unsigned int swapChainImageCount, vulkanDevice &device, vulkanDescriptorPool &descriptorPool, std::vector<VkDescriptorSetLayout> &layouts);
            void cleanup();

            void bindImage(const vulkanImageView &imageView, const vulkanSampler &imageSampler);
            void bindUBO(const vulkanBuffer &buffer, VkDeviceSize range);
            void update();

            bool needsUpdate() const;

            vulkanDescriptorSet *getDescriptorSet(unsigned int index);
            const vulkanDescriptorSet *getDescriptorSet(unsigned int index) const;

    };