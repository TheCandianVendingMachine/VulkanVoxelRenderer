#include "graphics/computePipeline.hpp"
#include "graphics/vulkan/vulkanDevice.hpp"

void computePipeline::create(vulkanDevice &device, unsigned int swapChainImageCount, descriptorSettings &settings, const VkPipelineShaderStageCreateInfo &shaderStage)
    {
        m_descriptorHandler.create(device, swapChainImageCount, settings);
        m_pipelineLayout.create(device, { m_descriptorHandler.getDescriptorSetLayout() });
        m_computePipeline.create(device, shaderStage, m_pipelineLayout, VK_NULL_HANDLE, 0);
    }

void computePipeline::cleanup()
    {
        m_computePipeline.cleanup();
        m_pipelineLayout.cleanup();
        m_descriptorHandler.cleanup();
    }
