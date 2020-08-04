#include "graphics/descriptorHandler.hpp"
#include "graphics/vulkan/vulkanDevice.hpp"
#include <vector>

void descriptorHandler::create(vulkanDevice &device, unsigned int swapChainImageCount, descriptorSettings &settings)
    {
        m_settings = std::move(settings);
        m_descriptorSetLayout.create(device, m_settings.getLayoutBindings());

        std::vector<VkDescriptorPoolSize> poolSizes(1);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImageCount); // how many of this type to allocate
        m_descriptorPool.create(device, c_maxSets * swapChainImageCount, m_settings.getPoolSizes(swapChainImageCount));

        m_device = &device;
        m_swapChainImageCount = swapChainImageCount;
    }

void descriptorHandler::cleanup()
    {
        for (auto &allocatedDescriptorSet : m_allocatedDescriptorSets)
            {
                allocatedDescriptorSet->cleanup();
            }
        m_descriptorPool.cleanup();
        m_descriptorSetLayout.cleanup();
    }

descriptorSet *descriptorHandler::createDescriptorSet()
    {
        descriptorSet *returnDescriptorSets;
        std::vector<VkDescriptorSetLayout> layouts(m_swapChainImageCount, m_descriptorSetLayout);

        m_allocatedDescriptorSets.emplace_back(std::make_unique<descriptorSet>());
        returnDescriptorSets = m_allocatedDescriptorSets.back().get();
        returnDescriptorSets->create(m_swapChainImageCount, *m_device, m_descriptorPool, layouts, m_settings);

        return returnDescriptorSets;
    }

vulkanDescriptorSetLayout &descriptorHandler::getDescriptorSetLayout()
    {
        return m_descriptorSetLayout;
    }

const vulkanDescriptorPool &descriptorHandler::getDescriptorPool() const
    {
        return m_descriptorPool;
    }

vulkanDescriptorPool &descriptorHandler::getDescriptorPool()
    {
        return m_descriptorPool;
    }

