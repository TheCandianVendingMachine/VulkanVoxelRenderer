#include "graphics/renderer.hpp"
#include "graphics/window.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex.hpp"
#include "graphics/vertexBuffer.hpp"
#include "graphics/vulkan/vulkanQueueFamilyIndices.hpp"
#include "graphics/vulkan/vulkanHelpers.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "voxel/voxelSpace.hpp"
#include <memory>

#include "optick.h"

void renderer::recordSubmissionCommandBuffer(VkCommandBuffer submissionBuffer)
    {
        OPTICK_EVENT("Record Submission Command Buffer", Optick::Category::Rendering);
        vkResetCommandBuffer(submissionBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(submissionBuffer, &beginInfo) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass.getRenderPass();
        renderPassInfo.framebuffer = m_renderPass.getFrameBuffers().at(m_frame);
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain.getExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 1.f, 1.f, 1.f, 1.f };
        clearValues[1].depthStencil = { 1, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        while (!m_queuedComputeDispatches.empty())
            {
                vkCmdExecuteCommands(submissionBuffer, 1, &m_computePipelines[m_queuedComputeDispatches.front()].m_commandBuffer.m_commandBuffer.getUnderlyingCommandBuffer());
                m_queuedComputeDispatches.pop();
            }
        
        vkCmdBeginRenderPass(submissionBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vkCmdExecuteCommands(submissionBuffer, 1, &m_commandBuffers[m_frame].m_commandBuffer.getUnderlyingCommandBuffer());
        vkCmdEndRenderPass(submissionBuffer);

        if (vkEndCommandBuffer(submissionBuffer) != VK_SUCCESS)
            {
                // <error>
                return;
            }
    }

renderer::renderer(window &app, descriptorSettings &settings)
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Voxels!";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        m_instance.create(appInfo, extensions);

        glfwCreateWindowSurface(m_instance, app.getUnderlyingWindow(), nullptr, &m_windowSurface);
        
        m_physicalDevice.create(m_instance, m_windowSurface);
        m_device.create(m_physicalDevice, m_windowSurface);

        m_allocator.create(m_instance, m_device, m_physicalDevice);
        globals::g_vulkanAllocator = &m_allocator;

        m_swapChain.create(m_device, m_physicalDevice, m_windowSurface, *app.getUnderlyingWindow());
        m_renderPass.create(m_device, m_physicalDevice, m_swapChain, [] (std::vector<vulkanSubpass> &subpasses, vulkanAttachmentList &attachmentList) {});

        shader frag;
        shader vert;

        frag.load(m_device, "shaders/raytracing_fragment.spv");
        vert.load(m_device, "shaders/raytracing_vertex.spv");

        m_surface.create(m_device, m_swapChain, m_renderPass.getRenderPass(), settings, 
            [this, &frag, &vert](std::vector<VkPipelineShaderStageCreateInfo> &shaderStages, VkPipelineVertexInputStateCreateInfo &vertexInputInfo, VkPipelineInputAssemblyStateCreateInfo &inputAssembly, VkPipelineTessellationStateCreateInfo&, VkPipelineMultisampleStateCreateInfo &multisampling, VkPipelineDepthStencilStateCreateInfo &depthStencil, VkPipelineDynamicStateCreateInfo&){
                VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module = vert.getShaderModule();
                vertShaderStageInfo.pName = vert.getEntryPoint().c_str();

                VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module = frag.getShaderModule();
                fragShaderStageInfo.pName = frag.getEntryPoint().c_str();

                shaderStages.push_back(vertShaderStageInfo);
                shaderStages.push_back(fragShaderStageInfo);

                static auto bindingDescription = vertex::getBindingDescription();
                static auto attributeDescriptions = vertex::getAttributeDescription();
                vertexInputInfo.vertexBindingDescriptionCount = 0;
                vertexInputInfo.pVertexBindingDescriptions = nullptr;
                vertexInputInfo.vertexAttributeDescriptionCount = 0;
                vertexInputInfo.pVertexAttributeDescriptions = nullptr;

                inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                inputAssembly.primitiveRestartEnable = VK_FALSE;

                multisampling.sampleShadingEnable = VK_TRUE;
                multisampling.rasterizationSamples = m_physicalDevice.getSampleCount();
                multisampling.minSampleShading = 0.2f;
                multisampling.pSampleMask = nullptr;
                multisampling.alphaToCoverageEnable = VK_FALSE;
                multisampling.alphaToOneEnable = VK_FALSE;

                depthStencil.depthWriteEnable = VK_TRUE;
                depthStencil.depthTestEnable = VK_TRUE;
                depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
                depthStencil.depthBoundsTestEnable = VK_FALSE;
                depthStencil.minDepthBounds = 0.f;
                depthStencil.maxDepthBounds = 1.f;
                depthStencil.stencilTestEnable = VK_FALSE;
                depthStencil.front = {};
                depthStencil.back = {};
            });
    
        vulkanQueueFamilyIndices queueFamilies = helpers::findQueueFamilies(m_physicalDevice, m_windowSurface);
        m_commandPool.create(m_device, queueFamilies);

        m_commandBuffers.resize(m_swapChain.getImageViews().size());
        for (auto &commandBuffer : m_commandBuffers)
            {
                commandBuffer.m_commandBuffer.create(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
            }

        m_submissionCommandBuffers.resize(m_swapChain.getImageViews().size());
        vulkanCommandBufferFunctions::createBatch(m_submissionCommandBuffers, m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        vkGetDeviceQueue(m_device, queueFamilies.m_graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, queueFamilies.m_presentFamily.value(), 0, &m_presentQueue);

        m_imageAvailableSemaphores.resize(c_maxFramesInFlight);
        m_renderFinishedSemaphores.resize(c_maxFramesInFlight);
        m_inFlightFences.resize(c_maxFramesInFlight);
        m_imagesInFlight.resize(m_swapChain.getImageViews().size());

        for (int i = 0; i < c_maxFramesInFlight; i++)
            {
                m_imageAvailableSemaphores[i].create(m_device);
                m_renderFinishedSemaphores[i].create(m_device);
                m_inFlightFences[i].create(m_device, VK_FENCE_CREATE_SIGNALED_BIT);
            }

        VkQueue queueArray[] = {
            m_graphicsQueue
        };

        uint32_t queueFamiliesArray[] = {
            queueFamilies.m_graphicsFamily.value()
        };

        //OPTICK_GPU_INIT_VULKAN(&m_device.getUnderlyingDevice(), &m_physicalDevice.getUnderlyingPhysicalDevice(), queueArray, queueFamiliesArray, 1);
    }

renderer::~renderer()
    {
        cleanup();
    }

void renderer::initImGui(window &app)
    {
        vulkanQueueFamilyIndices queueFamilies = helpers::findQueueFamilies(m_physicalDevice, m_windowSurface);

        descriptorSettings imGuiDescriptorSettings;
        imGuiDescriptorSettings.addSetting(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1);

        m_imGuiDescriptors.create(m_device, static_cast<unsigned int>(m_swapChain.getImageViews().size()), imGuiDescriptorSettings);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(app.getUnderlyingWindow(), true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = m_instance;
        initInfo.PhysicalDevice = m_physicalDevice;
        initInfo.Device = m_device;
        initInfo.QueueFamily = queueFamilies.m_graphicsFamily.value();
        initInfo.Queue = m_graphicsQueue;
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = m_imGuiDescriptors.getDescriptorPool();
        initInfo.MinImageCount = c_maxFramesInFlight;
        initInfo.ImageCount = c_maxFramesInFlight;
        initInfo.MSAASamples = m_physicalDevice.getSampleCount();
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&initInfo, m_renderPass.getRenderPass());

        // create fonts
        {
            vulkanCommandBuffer commandBuffer(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(commandBuffer, &info);

            ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer.getUnderlyingCommandBuffer();

            vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_graphicsQueue);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }

        m_imGuiEnabled = true;
    }

void renderer::deinitImGui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_imGuiDescriptors.cleanup();
        m_imGuiEnabled = false;
    }

void renderer::updateImGui()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

void renderer::cleanup()
    {
        for (unsigned int i = 0; i < c_maxFramesInFlight; i++)
            {
                m_inFlightFences[i].cleanup();
                m_renderFinishedSemaphores[i].cleanup();
                m_imageAvailableSemaphores[i].cleanup();
            }

        m_oneTimeBuffer.cleanup();

        for (auto &commandBuffer : m_submissionCommandBuffers)
            {
                commandBuffer.cleanup();
            }
        for (auto &commandBuffer : m_commandBuffers)
            {
                commandBuffer.m_commandBuffer.cleanup();
            }

        for (auto &computePipeline : m_computePipelines)
            {
                computePipeline.m_computePipeline.cleanup();
                computePipeline.m_commandBuffer.m_commandBuffer.cleanup();
            }

        m_commandPool.cleanup();
        m_surface.cleanup();
        m_renderPass.cleanup();
        m_swapChain.cleanup();
        m_allocator.cleanup();
        m_device.cleanup();
        m_physicalDevice.cleanup();
        if (m_windowSurface != VK_NULL_HANDLE)
            {
                vkDestroySurfaceKHR(m_instance, m_windowSurface, nullptr);
                m_windowSurface = VK_NULL_HANDLE;
            }
        m_instance.cleanup();
    }

unsigned int renderer::createComputePipeline(descriptorSettings &settings, const char *shaderPath)
    {
        shader computeShader;
        computeShader.load(m_device, shaderPath);

        VkPipelineShaderStageCreateInfo computeShaderInfo{};
        computeShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderInfo.module = computeShader.getShaderModule();
        computeShaderInfo.pName = computeShader.getEntryPoint().c_str();

        m_computePipelines.emplace_back();
        m_computePipelines.back().m_computePipeline.create(m_device, m_swapChain.getImageViews().size(), settings, computeShaderInfo);
        m_computePipelines.back().m_commandBuffer.m_commandBuffer.create(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

        return m_computePipelines.size() - 1;
    }

void renderer::dispatchCompute(unsigned int pipelineIndex, descriptorSet *descriptorSet, unsigned int x, unsigned int y, unsigned int z)
    {
        if (descriptorSet->needsUpdate())
            {
                descriptorSet->update();
            }

        computePipelineInternal &pipeline = m_computePipelines[pipelineIndex];
        if (!(pipeline.m_groupX == x && pipeline.m_groupY == y && pipeline.m_groupZ == z && pipeline.m_boundDescriptorSet == descriptorSet && pipeline.m_commandBuffer.m_compiled))
            {
                vulkanCommandBuffer commandBuffer = pipeline.m_commandBuffer.m_commandBuffer;
                vkResetCommandBuffer(commandBuffer, 0);

                VkCommandBufferInheritanceInfo inheritanceInfo{};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.pNext = nullptr;
                inheritanceInfo.renderPass = VK_NULL_HANDLE;
                inheritanceInfo.subpass = VK_NULL_HANDLE;
                inheritanceInfo.framebuffer = VK_NULL_HANDLE;
                inheritanceInfo.occlusionQueryEnable = VK_FALSE;
                inheritanceInfo.queryFlags = 0;
                inheritanceInfo.pipelineStatistics = VK_NULL_HANDLE;

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo;

                if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
                    {
                        // <error>
                        return;
                    }

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.m_computePipeline.m_computePipeline);
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.m_computePipeline.m_pipelineLayout, 0, 1, &descriptorSet->getDescriptorSet(m_frame)->getUnderlyingDescriptorSet(), 0, nullptr);
                vkCmdDispatch(commandBuffer, x, y, z);

                vkEndCommandBuffer(commandBuffer);

                pipeline.m_groupX = x;
                pipeline.m_groupY = y;
                pipeline.m_groupZ = z;
                pipeline.m_boundDescriptorSet = descriptorSet;
                pipeline.m_commandBuffer.m_compiled = true;
            }

        m_queuedComputeDispatches.push(pipelineIndex);
    }

void renderer::draw(descriptorSet &descriptorSet)
    {
        OPTICK_EVENT("Draw", Optick::Category::Rendering);
        m_renderables.push_back({ &descriptorSet});
    }

void renderer::preRecording()
    {
        OPTICK_EVENT("Pre-Record", Optick::Category::Rendering);
        {
            OPTICK_EVENT("wait for fence");
            m_inFlightFences[m_frame].wait();
        }
        vulkanCommandBuffer updateCommandBuffer;
        bool hasUpdate = false;

        for (auto &renderable : m_renderables)
            {
                OPTICK_EVENT("Update Renderable");
                if (renderable.m_descriptorSet->needsUpdate())
                    {
                        renderable.m_descriptorSet->update();
                    }
            }

        if (hasUpdate)
            {
                OPTICK_EVENT("Finish Buffer Recording");
                vkEndCommandBuffer(updateCommandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &updateCommandBuffer.getUnderlyingCommandBuffer();

                OPTICK_EVENT("Submit Queue");
                vkResetFences(m_device, 1, &m_inFlightFences[m_frame].getUnderlyingFence());
                vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_frame]);
            }

        if (m_imGuiEnabled)
            {
                OPTICK_EVENT("ImGui Render");
                ImGui::Render();
                m_imGuiDrawData = ImGui::GetDrawData();
            }
    }

void renderer::recordCommandBuffer()
    {
        OPTICK_EVENT("Record", Optick::Category::Rendering);
        VkCommandBufferInheritanceInfo inheritanceInfo{};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.pNext = nullptr;
        inheritanceInfo.renderPass = m_renderPass.getRenderPass();
        inheritanceInfo.subpass = VK_NULL_HANDLE;
        inheritanceInfo.framebuffer = VK_NULL_HANDLE;
        inheritanceInfo.occlusionQueryEnable = VK_FALSE;
        inheritanceInfo.queryFlags = 0;
        inheritanceInfo.pipelineStatistics = VK_NULL_HANDLE;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = &inheritanceInfo;

        VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_frame].m_commandBuffer;
        vkResetCommandBuffer(currentCommandBuffer, 0);
        if (vkBeginCommandBuffer(currentCommandBuffer, &beginInfo) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_surface.m_graphicsPipeline);

        for (auto &renderable : m_renderables)
            {
                VkDeviceSize offsets[] = { 0 };
                vulkanDescriptorSet *descriptorSet = renderable.m_descriptorSet->getDescriptorSet(m_frame);

                vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_surface.m_pipelineLayout, 0, 1, &descriptorSet->getUnderlyingDescriptorSet(), 0, nullptr);
                vkCmdDraw(currentCommandBuffer, 3, 1, 0, 0);
            }

        if (m_imGuiEnabled)
            {
                ImGui_ImplVulkan_RenderDrawData(m_imGuiDrawData, currentCommandBuffer);
            }

        if (vkEndCommandBuffer(currentCommandBuffer) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_renderables.clear();
    }

void renderer::display()
    {
        OPTICK_EVENT("Display", Optick::Category::Rendering);
        {
            OPTICK_EVENT("wait for in-flight fence");
            m_inFlightFences[m_frame].wait();
        }

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_frame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                // framebuffer resized
                return;
            }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                // <error>
                return;
            }

        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
            {
                OPTICK_EVENT("wait for image fence");
                vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
            }
        m_imagesInFlight[imageIndex] = m_inFlightFences[m_frame];

        VkCommandBuffer submissionBuffer = m_submissionCommandBuffers[imageIndex];
        recordSubmissionCommandBuffer(submissionBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {
            m_imageAvailableSemaphores[m_frame]
        };

        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &submissionBuffer;

        VkSemaphore signalSemaphores[] = {
            m_renderFinishedSemaphores[m_frame]
        };

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        {
            OPTICK_EVENT("Submit Graphics Queue");
            vkResetFences(m_device, 1, &m_inFlightFences[m_frame].getUnderlyingFence());
            if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_frame]) != VK_SUCCESS)
                {
                    // <error>
                    return;
                }
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {
            m_swapChain
        };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        {
            OPTICK_CATEGORY("Present", Optick::Category::Wait);
            result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                // framebuffer resized
            }
        else if (result != VK_SUCCESS)
            {
                // <error>
                return;
            }

        m_frame = (m_frame + 1) % std::min(c_maxFramesInFlight, static_cast<unsigned int>(m_commandBuffers.size()));
    }

void renderer::waitForDeviceIdle()
    {
        OPTICK_EVENT("Wait For Device", Optick::Category::Wait);
        if (m_device != VK_NULL_HANDLE)
            {
                vkDeviceWaitIdle(m_device);
            }
    }

vulkanDevice &renderer::getDevice()
    {
        return m_device;
    }

const vulkanDevice &renderer::getDevice() const
    {
        return m_device;
    }

descriptorSet *renderer::createDescriptorSet()
    {
        return m_surface.m_descriptorHandler.createDescriptorSet();
    }

descriptorSet *renderer::createComputeDescriptorSet(unsigned int pipeline)
    {
        return m_computePipelines[pipeline].m_computePipeline.m_descriptorHandler.createDescriptorSet();
    }

glm::vec2 renderer::getSize() const
    {
        return { m_swapChain.getExtent().width, m_swapChain.getExtent().height };
    }

void renderer::transitionImageLayout(const vulkanImage &image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        m_oneTimeBuffer.create(m_device, m_commandPool);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = image.mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                            
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                            
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
        else
            {
                // <error>
                return;
            }

        vkCmdPipelineBarrier(
            m_oneTimeBuffer.m_commandBuffer.m_commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        m_oneTimeBuffer.destroy(m_graphicsQueue);

    }

void renderer::blitImage(vulkanImage &src, VkImageLayout srcLayout, vulkanImage &dst, VkImageLayout dstLayout, int width, int height)
    {
        m_oneTimeBuffer.create(m_device, m_commandPool);
        vulkanCommandBuffer &commandBuffer = m_oneTimeBuffer.m_commandBuffer.m_commandBuffer;

        VkImageMemoryBarrier barrierSrc{};
        barrierSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierSrc.image = src;
        barrierSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierSrc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrierSrc.subresourceRange.baseArrayLayer = 0;
        barrierSrc.subresourceRange.layerCount = 1;
        barrierSrc.subresourceRange.levelCount = 1;

        VkImageMemoryBarrier barrierDst{};
        barrierDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierDst.image = dst;
        barrierDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrierDst.subresourceRange.baseArrayLayer = 0;
        barrierDst.subresourceRange.layerCount = 1;
        barrierDst.subresourceRange.levelCount = 1;

        barrierSrc.subresourceRange.baseMipLevel = 0;
        barrierSrc.oldLayout = srcLayout;
        barrierSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrierSrc.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        barrierDst.subresourceRange.baseMipLevel = 0;
        barrierDst.oldLayout = dstLayout;
        barrierDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierDst.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrierDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrierSrc
        );

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrierDst
        );

        VkImageBlit blitProperties{};
        blitProperties.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitProperties.srcSubresource.baseArrayLayer = 0;
        blitProperties.srcSubresource.mipLevel = 0;
        blitProperties.srcSubresource.layerCount = 1;

        blitProperties.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitProperties.dstSubresource.baseArrayLayer = 0;
        blitProperties.dstSubresource.mipLevel = 0;
        blitProperties.dstSubresource.layerCount = 1;

        blitProperties.srcOffsets[0] = { 0, 0, 0 };
        blitProperties.srcOffsets[1] = { width, height, 1 };

        blitProperties.dstOffsets[0] = { 0, 0, 0 };
        blitProperties.dstOffsets[1] = { width, height, 1 };

        vkCmdBlitImage(commandBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitProperties, VK_FILTER_LINEAR);

        barrierSrc.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrierSrc.newLayout = srcLayout;
        barrierSrc.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrierSrc.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        barrierDst.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierDst.newLayout = dstLayout;
        barrierDst.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrierDst.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrierSrc
        );

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrierDst
        );

        m_oneTimeBuffer.destroy(m_graphicsQueue);
    }

void renderer::oneTimeCommandBuffer::create(const vulkanDevice &device, const vulkanCommandPool &commandPool)
    {
        if (!m_created)
            {
                while (m_executing) {}
                m_commandBuffer.m_commandBuffer.create(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                vkBeginCommandBuffer(m_commandBuffer.m_commandBuffer, &beginInfo);

                m_created = true;
            }
    }

void renderer::oneTimeCommandBuffer::destroy(VkQueue queue)
    {
        if (m_created)
            {
                m_executing = true;

                //std::thread endThread([this, queue](){
                    vkEndCommandBuffer(m_commandBuffer.m_commandBuffer);

                    VkSubmitInfo submitInfo{};
                    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &m_commandBuffer.m_commandBuffer.getUnderlyingCommandBuffer();

                    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
                    vkQueueWaitIdle(queue);

                    m_commandBuffer.m_commandBuffer.cleanup();
                    m_executing = false;
                //});
                //endThread.detach();

                m_created = false;
            }
    }

void renderer::oneTimeCommandBuffer::cleanup()
    {
        m_commandBuffer.m_commandBuffer.cleanup();
        m_created = false;
        m_executing = false;
    }
