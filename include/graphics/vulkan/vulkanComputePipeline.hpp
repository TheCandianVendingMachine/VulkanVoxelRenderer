// vulkanComputePipeline.hpp
// A wrapper for VkPipeline specifically for compute shaders
#include <vulkan/vulkan.h>
#include <vector>

class vulkanComputePipeline;
namespace vulkanComputePipelineFunctions
    {
        void createBatch(std::vector<vulkanComputePipeline> &graphicsPipelines, VkDevice device, const std::vector<VkGraphicsPipelineCreateInfo> &createInfo);
    }

class vulkanComputePipeline
    {
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            VkPipeline m_computePipeline = VK_NULL_HANDLE;

            bool m_cleanedUp = true;

            friend void vulkanComputePipelineFunctions::createBatch(std::vector<vulkanComputePipeline>&, VkDevice, const std::vector<VkGraphicsPipelineCreateInfo>&);

        public:
            vulkanComputePipeline() = default;
            vulkanComputePipeline(
                VkDevice device,
                const VkPipelineShaderStageCreateInfo &shaderStage,
                VkPipelineLayout layout,
                VkPipeline basePipeline, int32_t pipelineIndex
            );
            ~vulkanComputePipeline();
            void create(
                VkDevice device,
                const VkPipelineShaderStageCreateInfo &shaderStage,
                VkPipelineLayout layout,
                VkPipeline basePipeline, int32_t pipelineIndex
            );
            void cleanup();

            bool isCreated() const;

            VkPipeline getUnderlyingPipeline() const;
            operator VkPipeline() const;

            VkPipeline &getUnderlyingPipeline();

    };
