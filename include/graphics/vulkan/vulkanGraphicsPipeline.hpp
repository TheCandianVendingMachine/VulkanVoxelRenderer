// vulkanGraphicsPipeline.hpp
// A wrapper for VkPipeline specifically for graphics pushing
#include <vulkan/vulkan.h>
#include <vector>

class vulkanGraphicsPipeline;
namespace vulkanGraphicsPipelineFunctions
    {
        void createBatch(std::vector<vulkanGraphicsPipeline> &graphicsPipelines, VkDevice device, const std::vector<VkGraphicsPipelineCreateInfo> &createInfo);
    }

class vulkanGraphicsPipeline
    {
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

            friend void vulkanGraphicsPipelineFunctions::createBatch(std::vector<vulkanGraphicsPipeline>&, VkDevice, const std::vector<VkGraphicsPipelineCreateInfo>&);

        public:
            vulkanGraphicsPipeline() = default;
            vulkanGraphicsPipeline(
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
            );
            ~vulkanGraphicsPipeline();
            void create(
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
            );
            void cleanup();

            VkPipeline getUnderlyingPipeline() const;
            operator VkPipeline() const;

            VkPipeline &getUnderlyingPipeline();

    };
