#include "graphics/descriptorHandler.hpp"
#include "graphics/vulkan/vulkanDevice.hpp"
#include <vector>

void descriptorHandler::create(vulkanDevice &device, unsigned int swapChainImageCount)
    {
        // if you change the order of descriptors here, change it in descriptorSet as well
        VkDescriptorSetLayoutBinding mvpBinding{};
        mvpBinding.binding = 0;
        mvpBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpBinding.descriptorCount = 1;
        mvpBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        mvpBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding transformBinding{};
        transformBinding.binding = 1;
        transformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        transformBinding.descriptorCount = 1;
        transformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        transformBinding.pImmutableSamplers = nullptr;

        m_descriptorSetLayout.create(device, { mvpBinding, transformBinding });

        std::vector<VkDescriptorPoolSize> poolSizes(2);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImageCount); // how many of this type to allocate
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImageCount); // how many of this type to allocate
        m_descriptorPool.create(device, c_maxSets * swapChainImageCount, poolSizes);

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
        returnDescriptorSets->create(m_swapChainImageCount, *m_device, m_descriptorPool, layouts);

        return returnDescriptorSets;
    }

vulkanDescriptorSetLayout &descriptorHandler::getDescriptorSetLayout()
    {
        return m_descriptorSetLayout;
    }

