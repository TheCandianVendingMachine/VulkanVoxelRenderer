#include "graphics/descriptorSet.hpp"
#include "graphics/vulkan/vulkanDevice.hpp"
#include "graphics/vulkan/vulkanDescriptorPool.hpp"
#include "graphics/vulkan/vulkanImageView.hpp"
#include "graphics/vulkan/vulkanSampler.hpp"
#include "graphics/vulkan/vulkanBuffer.hpp"
#include "graphics/descriptorSettings.hpp"
#include <algorithm>

void descriptorSet::sortBindings()
    {
        std::sort(m_bindings.begin(), m_bindings.end(), [](const metaInfo &a, const metaInfo &b){
            return a.binding < b.binding;
        });
    }

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

void descriptorSet::bindImage(const vulkanImageView &imageView, const vulkanSampler &imageSampler, int binding)
    {
        VkDescriptorImageInfo newInfo{};

        newInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        newInfo.imageView = imageView;
        newInfo.sampler = imageSampler;

        if (binding >= m_bindings.size())
            {
                m_bindings.emplace_back(binding);
                sortBindings();
            }

        m_bindings[binding].imageInfo.m_imageInfo = { newInfo };

        m_needsUpdate = true;
    }

void descriptorSet::bindImages(const vulkanImageView *imageViews, const vulkanSampler *imageSamplers, int count, int binding)
    {
        if (binding >= m_bindings.size())
            {
                m_bindings.emplace_back(binding);
                sortBindings();
            }
        m_bindings[binding].imageInfo.m_imageInfo.clear();
        
        for (int i = 0; i < count; i++)
            {
                VkDescriptorImageInfo newInfo{};
                newInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                newInfo.imageView = imageViews[i];
                newInfo.sampler = imageSamplers[i];

                m_bindings[binding].imageInfo.m_imageInfo.push_back(newInfo);
            }

        m_needsUpdate = true;
    }

void descriptorSet::bindImage(const vulkanImageView &imageView, int binding)
    {
        VkDescriptorImageInfo newInfo{};

        newInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        newInfo.imageView = imageView;

        if (binding >= m_bindings.size())
            {
                m_bindings.emplace_back(binding);
                sortBindings();
            }
        m_bindings[binding].imageInfo.m_imageInfo = { newInfo };

        m_needsUpdate = true;
    }

void descriptorSet::bindUBO(const vulkanBuffer &buffer, VkDeviceSize range, int binding)
    {
        VkDescriptorBufferInfo newInfo{};

        newInfo.buffer = buffer;
        newInfo.offset = 0;
        newInfo.range = range;

        if (binding >= m_bindings.size())
            {
                m_bindings.emplace_back(binding);
                sortBindings();
            }
        m_bindings[binding].bufferInfo.m_bufferInfo = { newInfo };

        m_needsUpdate = true;
    }

void descriptorSet::bindSBO(const vulkanBuffer &buffer, VkDeviceSize range, int binding)
    {
        VkDescriptorBufferInfo newInfo{};

        newInfo.buffer = buffer;
        newInfo.offset = 0;
        newInfo.range = range;

        if (binding >= m_bindings.size())
            {
                m_bindings.emplace_back(binding);
                sortBindings();
            }
        m_bindings[binding].bufferInfo.m_bufferInfo = { newInfo };

        m_needsUpdate = true;
    }

void descriptorSet::update()
    {
        // to add more descriptor sets you have to add them in descriptorHandler too

        for (unsigned int i = 0; i < m_descriptorSets.size(); i++)
            {
                VkDescriptorSet descriptorSet = m_descriptorSets[i];

                std::vector<VkWriteDescriptorSet> descriptorWrites = m_settings->getDescriptorWrites();
                unsigned int binding = 0;
                for (auto &write : descriptorWrites)
                    {
                        write.dstSet = descriptorSet;
                        switch (write.descriptorType)
                            {
                                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                                    write.descriptorCount = m_bindings[binding].bufferInfo.m_bufferInfo.size();
                                    write.pBufferInfo = m_bindings[binding].bufferInfo.m_bufferInfo.data();

                                    binding++;
                                    break;
                                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                                    for (auto &imageInfo : m_bindings[binding].imageInfo.m_imageInfo)
                                        {
                                            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                                        }
                                    [[fallthrough]];
                                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                                    write.descriptorCount = m_bindings[binding].imageInfo.m_imageInfo.size();
                                    write.pImageInfo = m_bindings[binding].imageInfo.m_imageInfo.data();

                                    binding++;
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
