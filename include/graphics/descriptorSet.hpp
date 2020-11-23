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
                    explicit metaImageInfo(VkDescriptorImageInfo info)
                        {
                            m_imageInfo.push_back(info);
                        }
                };

            struct metaBufferInfo
                {
                    std::vector<VkDescriptorBufferInfo> m_bufferInfo;

                    metaBufferInfo() = default;
                    explicit metaBufferInfo(VkDescriptorBufferInfo info)
                        {
                            m_bufferInfo.push_back(info);
                        }
                };

            struct metaInfo
                {
                    int binding = -1;
                    metaImageInfo imageInfo;
                    metaBufferInfo bufferInfo;

                    metaInfo(int binding) : binding(binding) {}
                };

            std::vector<vulkanDescriptorSet> m_descriptorSets;
            std::vector<metaInfo> m_bindings;

            bool m_needsUpdate = false;

            const descriptorSettings *m_settings = nullptr;
            vulkanDevice *m_device = nullptr;

            void sortBindings();

        public:
            void create(unsigned int swapChainImageCount, vulkanDevice &device, vulkanDescriptorPool &descriptorPool, std::vector<VkDescriptorSetLayout> &layouts, const descriptorSettings &settings);
            void cleanup();

            void bindImage(const vulkanImageView &imageView, const vulkanSampler &imageSampler, int binding);
            void bindImages(const vulkanImageView *imageViews, const vulkanSampler *imageSamplers, int count, int binding);
            void bindImage(const vulkanImageView &imageView, int binding);
            void bindUBO(const vulkanBuffer &buffer, VkDeviceSize range, int binding);
            void bindSBO(const vulkanBuffer &buffer, VkDeviceSize range, int binding);
            void update();

            bool needsUpdate() const;

            vulkanDescriptorSet *getDescriptorSet(unsigned int index);
            const vulkanDescriptorSet *getDescriptorSet(unsigned int index) const;

    };
