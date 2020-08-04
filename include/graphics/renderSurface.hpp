// renderSurface.hpp
// A surface which is rendered to. Contains a render pass and swapchain
#pragma once
#include <functional>
#include <vector>
#include "graphics/vulkan/vulkanGraphicsPipeline.hpp"
#include "graphics/vulkan/vulkanPipelineLayout.hpp"
#include "graphics/vulkan/vulkanDescriptorSetLayout.hpp"
#include "graphics/descriptorHandler.hpp"

class vulkanSwapChain;
class vulkanDevice;
class vulkanRenderPass;
class descriptorSettings;
class renderSurface
    {
        private:
            vulkanGraphicsPipeline m_graphicsPipeline;
            vulkanPipelineLayout m_pipelineLayout;
            descriptorHandler m_descriptorHandler;

            friend class renderer;

        public:
            void create(
                vulkanDevice &device,
                vulkanSwapChain &swapChain,
                vulkanRenderPass &renderPass,
                descriptorSettings &settings,
                std::function<void(std::vector<VkPipelineShaderStageCreateInfo>&, VkPipelineVertexInputStateCreateInfo&, VkPipelineInputAssemblyStateCreateInfo&, VkPipelineTessellationStateCreateInfo&, VkPipelineMultisampleStateCreateInfo&, VkPipelineDepthStencilStateCreateInfo&, VkPipelineDynamicStateCreateInfo&)> graphicsPipelineInit
            );
            void cleanup();
    };
