// computePipeline.hpp
// a high-level interface to define and use compute shaders easily
#pragma once
#include "graphics/vulkan/vulkanComputePipeline.hpp"
#include "graphics/vulkan/vulkanPipelineLayout.hpp"
#include "graphics/descriptorHandler.hpp"

class computePipeline
    {
        private:
            vulkanComputePipeline m_computePipeline;
            vulkanPipelineLayout m_pipelineLayout;
            descriptorHandler m_descriptorHandler;

            friend class renderer;

        public:
            void create(vulkanDevice &device, unsigned int swapChainImageCount, descriptorSettings &settings, const VkPipelineShaderStageCreateInfo &shaderStage);
            void cleanup();
    };
