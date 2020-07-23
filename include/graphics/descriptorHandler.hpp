// descriptorHandler.hpp
// Handles description sets for a pipeline
#pragma once
#include <vector>
#include <memory>
#include "descriptorSet.hpp"
#include "graphics/vulkan/vulkanDescriptorSetLayout.hpp"
#include "graphics/vulkan/vulkanDescriptorPool.hpp"

class vulkanDevice;
class descriptorHandler
    {
        private:
            static constexpr unsigned int c_maxSets = 10;
            unsigned int m_swapChainImageCount = 0;

            vulkanDescriptorSetLayout m_descriptorSetLayout;
            vulkanDescriptorPool m_descriptorPool;
            vulkanDevice *m_device = nullptr;

            std::vector<std::unique_ptr<descriptorSet>> m_allocatedDescriptorSets;

        public:
            void create(vulkanDevice &device, unsigned int swapChainImageCount);
            void cleanup();

            descriptorSet *createDescriptorSet();
            vulkanDescriptorSetLayout &getDescriptorSetLayout();

    };
