#include "graphics/vulkan/vulkanComputePipeline.hpp"

vulkanComputePipeline::vulkanComputePipeline(VkDevice device, const VkPipelineShaderStageCreateInfo &shaderStage, VkPipelineLayout layout, VkPipeline basePipeline, int32_t pipelineIndex)
    {
        create(device, shaderStage, layout, basePipeline, pipelineIndex);
    }

vulkanComputePipeline::~vulkanComputePipeline()
    {
        cleanup();
    }

void vulkanComputePipeline::create(VkDevice device, const VkPipelineShaderStageCreateInfo &shaderStage, VkPipelineLayout layout, VkPipeline basePipeline, int32_t pipelineIndex)
    {
        VkComputePipelineCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.stage = shaderStage;
        createInfo.layout = layout;
        createInfo.basePipelineHandle = basePipeline;
        createInfo.basePipelineIndex = pipelineIndex;

        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_computePipeline) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanComputePipeline::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyPipeline(m_device, m_computePipeline, nullptr);
        m_computePipeline = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

VkPipeline vulkanComputePipeline::getUnderlyingPipeline() const
    {
        return m_computePipeline;
    }

vulkanComputePipeline::operator VkPipeline() const
    {
        return m_computePipeline;
    }

VkPipeline &vulkanComputePipeline::getUnderlyingPipeline()
    {
        return m_computePipeline;
    }

void vulkanComputePipelineFunctions::createBatch(std::vector<vulkanComputePipeline> &graphicsPipelines, VkDevice device, const std::vector<VkGraphicsPipelineCreateInfo> &createInfo)
    {
        std::vector<VkPipeline> graphicsPipelinesRaw(graphicsPipelines.begin(), graphicsPipelines.end());
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, static_cast<uint32_t>(createInfo.size()), createInfo.data(), nullptr, graphicsPipelinesRaw.data()) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        unsigned int i = 0;
        for (auto &graphicsPipeline : graphicsPipelines)
            {
                graphicsPipeline.m_computePipeline = graphicsPipelinesRaw[i++];
                graphicsPipeline.m_device = device;
                graphicsPipeline.m_cleanedUp = false;
            }
    }
