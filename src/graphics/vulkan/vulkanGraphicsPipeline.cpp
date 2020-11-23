#include "graphics/vulkan/vulkanGraphicsPipeline.hpp"

vulkanGraphicsPipeline::vulkanGraphicsPipeline(
    VkDevice device, 
    const std::vector<VkPipelineShaderStageCreateInfo> &shaderStages, 
    VkPipelineVertexInputStateCreateInfo vertexInputState, 
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState, 
    VkPipelineTessellationStateCreateInfo tesselationState, 
    VkPipelineViewportStateCreateInfo viewportState, 
    VkPipelineRasterizationStateCreateInfo rasterizationState,
    VkPipelineMultisampleStateCreateInfo multisampleState, 
    VkPipelineDepthStencilStateCreateInfo depthStencilState, 
    VkPipelineColorBlendStateCreateInfo colourBlendState, 
    VkPipelineDynamicStateCreateInfo dynamicState, 
    VkPipelineLayout layout, 
    VkRenderPass renderPass, 
    uint32_t subpass, 
    VkPipeline basePipeline, int32_t pipelineIndex
)
    {
        create(
            device, 
            shaderStages, 
            vertexInputState, 
            inputAssemblyState, 
            tesselationState, 
            viewportState, 
            rasterizationState, 
            multisampleState, 
            depthStencilState, 
            colourBlendState, 
            dynamicState, 
            layout, 
            renderPass, 
            subpass, 
            basePipeline, 
            pipelineIndex
        );
    }

vulkanGraphicsPipeline::~vulkanGraphicsPipeline()
    {
        cleanup();
    }

void vulkanGraphicsPipeline::create(
    VkDevice device,
    const std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
    VkPipelineVertexInputStateCreateInfo vertexInputState,
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState,
    VkPipelineTessellationStateCreateInfo tesselationState,
    VkPipelineViewportStateCreateInfo viewportState,
    VkPipelineRasterizationStateCreateInfo rasterizationState,
    VkPipelineMultisampleStateCreateInfo multisampleState,
    VkPipelineDepthStencilStateCreateInfo depthStencilState,
    VkPipelineColorBlendStateCreateInfo colourBlendState,
    VkPipelineDynamicStateCreateInfo dynamicState,
    VkPipelineLayout layout,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkPipeline basePipeline, int32_t pipelineIndex
) 
    {
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputState;
        pipelineInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineInfo.pTessellationState = &tesselationState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizationState;
        pipelineInfo.pMultisampleState = &multisampleState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pColorBlendState = &colourBlendState;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = layout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = subpass;
        pipelineInfo.basePipelineHandle = basePipeline;
        pipelineInfo.basePipelineIndex = pipelineIndex;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_device = device;
        m_cleanedUp = false;
    }

void vulkanGraphicsPipeline::cleanup()
    {
        if (m_cleanedUp) { return; }

        vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
        m_graphicsPipeline = VK_NULL_HANDLE;

        m_cleanedUp = true;
    }

bool vulkanGraphicsPipeline::isCreated() const
    {
        return !m_cleanedUp;
    }

VkPipeline vulkanGraphicsPipeline::getUnderlyingPipeline() const
    {
        return m_graphicsPipeline;
    }

vulkanGraphicsPipeline::operator VkPipeline() const
    {
        return m_graphicsPipeline;
    }

VkPipeline &vulkanGraphicsPipeline::getUnderlyingPipeline()
    {
        return m_graphicsPipeline;
    }

void vulkanGraphicsPipelineFunctions::createBatch(std::vector<vulkanGraphicsPipeline> &graphicsPipelines, VkDevice device, const std::vector<VkGraphicsPipelineCreateInfo> &createInfo)
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
                graphicsPipeline.m_graphicsPipeline = graphicsPipelinesRaw[i++];
                graphicsPipeline.m_device = device;
                graphicsPipeline.m_cleanedUp = false;
            }
    }
