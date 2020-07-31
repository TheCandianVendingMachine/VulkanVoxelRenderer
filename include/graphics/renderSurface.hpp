// renderSurface.hpp
// A surface which is rendered to. Contains a render pass and swapchain
#pragma once
#include <functional>
#include <vector>
#include "graphics/vulkan/vulkanSwapChain.hpp"
#include "graphics/vulkan/vulkanRenderPass.hpp"
#include "graphics/vulkan/vulkanGraphicsPipeline.hpp"
#include "graphics/vulkan/vulkanPipelineLayout.hpp"
#include "graphics/vulkan/vulkanDescriptorSetLayout.hpp"
#include "graphics/renderPass.hpp"
#include "graphics/descriptorHandler.hpp"

struct GLFWwindow;
class vulkanSubpass;
class vulkanAttachmentList;
class vulkanPhysicalDevice;
class vulkanDevice;
class descriptorSettings;
class renderSurface
    {
        private:
            vulkanSwapChain m_swapChain;
            renderPass m_renderPass;
            vulkanGraphicsPipeline m_graphicsPipeline;
            vulkanPipelineLayout m_pipelineLayout;
            descriptorHandler m_descriptorHandler;

            friend class renderer;

        public:
            void create(
                vulkanDevice &device,
                vulkanPhysicalDevice &physicalDevice, 
                VkSurfaceKHR surface, 
                const GLFWwindow &window,
                descriptorSettings &settings,
                std::function<void(std::vector<vulkanSubpass>&, vulkanAttachmentList&)> renderPassInit,
                std::function<void(std::vector<VkPipelineShaderStageCreateInfo>&, VkPipelineVertexInputStateCreateInfo&, VkPipelineInputAssemblyStateCreateInfo&, VkPipelineTessellationStateCreateInfo&, VkPipelineMultisampleStateCreateInfo&, VkPipelineDepthStencilStateCreateInfo&, VkPipelineDynamicStateCreateInfo&)> graphicsPipelineInit
            );
            void cleanup();
    };
