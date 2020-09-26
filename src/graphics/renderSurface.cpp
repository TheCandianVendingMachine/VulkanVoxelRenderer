#include "graphics/renderSurface.hpp"
#include "graphics/vulkan/vulkanDevice.hpp"
#include "graphics/vulkan/vulkanSwapChain.hpp"
#include "graphics/vulkan/vulkanRenderPass.hpp"

void renderSurface::create(
    vulkanDevice &device,
    vulkanSwapChain &swapChain,
    vulkanRenderPass &renderPass,
    descriptorSettings &settings,
    std::function<void(std::vector<VkPipelineShaderStageCreateInfo>&, VkPipelineVertexInputStateCreateInfo&, VkPipelineInputAssemblyStateCreateInfo&, VkPipelineTessellationStateCreateInfo&, VkPipelineMultisampleStateCreateInfo&, VkPipelineDepthStencilStateCreateInfo&, VkPipelineDynamicStateCreateInfo&)> graphicsPipelineInit
)
    {
        m_descriptorHandler.create(device, static_cast<unsigned int>(swapChain.getImageViews().size()), settings);

        m_pipelineLayout.create(device, { m_descriptorHandler.getDescriptorSetLayout() });

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineVertexInputStateCreateInfo vertexInputState{};        vertexInputState.sType =    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};    inputAssemblyState.sType =  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        VkPipelineTessellationStateCreateInfo tesselationState{};       tesselationState.sType =    VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        VkPipelineMultisampleStateCreateInfo multisampleState{};        multisampleState.sType =    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};      depthStencilState.sType =   VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        VkPipelineDynamicStateCreateInfo dynamicState{};                dynamicState.sType =        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

        graphicsPipelineInit(shaderStages, vertexInputState, inputAssemblyState, tesselationState, multisampleState, depthStencilState, dynamicState);

        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = static_cast<float>(swapChain.getExtent().width);
        viewport.height = static_cast<float>(swapChain.getExtent().height);
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChain.getExtent();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.f;
        rasterizer.depthBiasClamp = 0.f;
        rasterizer.depthBiasSlopeFactor = 0.f;

        VkPipelineColorBlendAttachmentState colourBlendAttachment{};
        colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colourBlendAttachment.blendEnable = VK_FALSE;
        colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        
        VkPipelineColorBlendStateCreateInfo colourBlending{};
        colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colourBlending.logicOpEnable = VK_FALSE;
        colourBlending.logicOp = VK_LOGIC_OP_COPY;
        colourBlending.attachmentCount = 1;
        colourBlending.pAttachments = &colourBlendAttachment;
        colourBlending.blendConstants[0] = 0.f;
        colourBlending.blendConstants[1] = 0.f;
        colourBlending.blendConstants[2] = 0.f;
        colourBlending.blendConstants[3] = 0.f;

        m_graphicsPipeline.create(
            device,
            shaderStages, 
            vertexInputState, 
            inputAssemblyState, 
            tesselationState, 
            viewportState, 
            rasterizer, 
            multisampleState, 
            depthStencilState, 
            colourBlending, 
            dynamicState, 
            m_pipelineLayout, 
            renderPass,
            0, VK_NULL_HANDLE, 0
        );
    }

void renderSurface::cleanup()
    {
        m_graphicsPipeline.cleanup();
        m_pipelineLayout.cleanup();
        m_descriptorHandler.cleanup();
    }
