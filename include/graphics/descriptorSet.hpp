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
class descriptorSettings;
class descriptorSet
    {
        private:
            struct metaImageInfo
            {
                std::vector<VkDescriptorImageInfo> m_imageInfo;
                metaImageInfo() = default;
                metaImageInfo(VkDescriptorImageInfo info)
                    {
                        m_imageInfo.push_back(info);
                    }
            };

            std::vector<vulkanDescriptorSet> m_descriptorSets;
            std::vector<VkDescriptorBufferInfo> m_bufferInfo;
            std::vector<metaImageInfo> m_imageInfo;
            bool m_needsUpdate = false;

            const descriptorSettings *m_settings = nullptr;
            vulkanDevice *m_device = nullptr;

        public:
            void create(unsigned int swapChainImageCount, vulkanDevice &device, vulkanDescriptorPool &descriptorPool, std::vector<VkDescriptorSetLayout> &layouts, const descriptorSettings &settings);
            void cleanup();

            void bindImage(const vulkanImageView &imageView, const vulkanSampler &imageSampler);
            void bindImages(const vulkanImageView *imageViews, const vulkanSampler *imageSamplers, int count);
            void bindImage(const vulkanImageView &imageView);
            void bindUBO(const vulkanBuffer &buffer, VkDeviceSize range);
            void bindSBO(const vulkanBuffer &buffer, VkDeviceSize range);
            void update();

            bool needsUpdate() const;

            vulkanDescriptorSet *getDescriptorSet(unsigned int index);
            const vulkanDescriptorSet *getDescriptorSet(unsigned int index) const;

    };
