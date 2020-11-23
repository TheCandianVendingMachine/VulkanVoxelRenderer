#include "graphics/descriptorSet.hpp"
#include "graphics/vulkan/vulkanDevice.hpp"
#include "graphics/vulkan/vulkanDescriptorPool.hpp"
#include "graphics/vulkan/vulkanImageView.hpp"
#include "graphics/vulkan/vulkanSampler.hpp"
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/descriptorSettings.hpp"
#include <array>

void descriptorSet::create(unsigned int swapChainImageCount, vulkanDevice &device, vulkanDescriptorPool &descriptorPool, std::vector<VkDescriptorSetLayout> &layouts, const descriptorSettings &settings)
    {
        m_descriptorSets.resize(swapChainImageCount);
        vulkanDescriptorSetFunctions::createBatch(m_descriptorSets, device, descriptorPool, layouts);
        m_device = &device;
        m_settings = &settings;
    }

void descriptorSet::cleanup()
    {
        for (auto &descriptorSet : m_descriptorSets)
            {
                descriptorSet.cleanup();
            }
    }

void descriptorSet::bindImage(const vulkanImageView &imageView, const vulkanSampler &imageSampler)
    {
        VkDescriptorImageInfo newInfo{};

        newInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        newInfo.imageView = imageView;
        newInfo.sampler = imageSampler;

        m_imageInfo.emplace_back(newInfo);

        m_needsUpdate = true;
    }

void descriptorSet::bindImages(const vulkanImageView *imageViews, const vulkanSampler *imageSamplers, int count)
    {
        m_imageInfo.emplace_back();
        for (int i = 0; i < count; i++)
            {
                VkDescriptorImageInfo newInfo{};
                newInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                newInfo.imageView = imageViews[i];
                newInfo.sampler = imageSamplers[i];

                m_imageInfo.back().m_imageInfo.push_back(newInfo);
            }

        m_needsUpdate = true;
    }

void descriptorSet::bindImage(const vulkanImageView &imageView)
    {
        VkDescriptorImageInfo newInfo{};

        newInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        newInfo.imageView = imageView;

        m_imageInfo.emplace_back(newInfo);

        m_needsUpdate = true;
    }

void descriptorSet::bindUBO(const vulkanBuffer &buffer, VkDeviceSize range)
    {
        VkDescriptorBufferInfo newInfo{};

        newInfo.buffer = buffer;
        newInfo.offset = 0;
        newInfo.range = range;

        m_bufferInfo.push_back(newInfo);
        
        m_needsUpdate = true;
    }

void descriptorSet::bindSBO(const vulkanBuffer &buffer, VkDeviceSize range)
    {
        VkDescriptorBufferInfo newInfo{};

        newInfo.buffer = buffer;
        newInfo.offset = 0;
        newInfo.range = range;

        m_bufferInfo.push_back(newInfo);
        
        m_needsUpdate = true;
    }

void descriptorSet::update()
    {
        // to add more descriptor sets you have to add them in descriptorHandler too
        for (unsigned int i = 0; i < m_descriptorSets.size(); i++)
            {
                VkDescriptorSet descriptorSet = m_descriptorSets[i];

                std::vector<VkWriteDescriptorSet> descriptorWrites = m_settings->getDescriptorWrites();
                unsigned int bufferIndex = 0;
                unsigned int imageIndex = 0;
                for (auto &write : descriptorWrites)
                    {
                        write.dstSet = descriptorSet;
                        switch (write.descriptorType)
                            {
                                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                                    write.pBufferInfo = &m_bufferInfo[bufferIndex++];
                                    break;
                                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                                    for (auto &imageInfo : m_imageInfo[imageIndex].m_imageInfo)
                                        {
                                            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                                        }
                                    [[fallthrough]];
                                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                                    write.descriptorCount = m_imageInfo[imageIndex].m_imageInfo.size();
                                    write.pImageInfo = m_imageInfo[imageIndex++].m_imageInfo.data();
                                    break;
                                default:
                                    break;
                            }
                    }

                vkUpdateDescriptorSets(m_device->getUnderlyingDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }

        m_needsUpdate = false;
    }

bool descriptorSet::needsUpdate() const
    {
        return m_needsUpdate;
    }

vulkanDescriptorSet *descriptorSet::getDescriptorSet(unsigned int index)
    {
        return &m_descriptorSets.at(index);
    }

const vulkanDescriptorSet *descriptorSet::getDescriptorSet(unsigned int index) const
    {
        return &m_descriptorSets.at(index);
    }
