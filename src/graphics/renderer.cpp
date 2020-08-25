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

        vkCmdBeginRenderPass(submissionBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vkCmdExecuteCommands(submissionBuffer, 1, &m_commandBuffers[m_frame].getUnderlyingCommandBuffer());
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

        frag.load(m_device, "shaders/frag.spv");
        vert.load(m_device, "shaders/vert.spv");

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
                vertexInputInfo.vertexBindingDescriptionCount = 1;
                vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
                vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
                vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
        vulkanCommandBufferFunctions::createBatch(m_commandBuffers, m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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
            m_graphicsQueue,
            m_presentQueue
        };

        uint32_t queueFamiliesArray[] = {
            queueFamilies.m_graphicsFamily.value(),
            queueFamilies.m_presentFamily.value()
        };

        OPTICK_GPU_INIT_VULKAN(&m_device.getUnderlyingDevice(), &m_physicalDevice.getUnderlyingPhysicalDevice(), queueArray, queueFamiliesArray, 1);
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

        for (auto &commandBuffer : m_submissionCommandBuffers)
            {
                commandBuffer.cleanup();
            }
        for (auto &commandBuffer : m_commandBuffers)
            {
                commandBuffer.cleanup();
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

void renderer::draw(descriptorSet &descriptorSet, voxelSpace &voxelSpace)
    {
        OPTICK_EVENT("Draw", Optick::Category::Rendering);
        m_renderables.push_back({ &descriptorSet, voxelSpace });
    }

void renderer::preRecording()
    {
        OPTICK_EVENT("Pre-Record", Optick::Category::Rendering);
        m_inFlightFences[m_frame].wait();
        vulkanCommandBuffer updateCommandBuffer;
        bool hasUpdate = false;

        for (auto &renderable : m_renderables)
            {
                OPTICK_EVENT("Update Renderable");
                if (renderable.m_voxelSpace.needsUpdate())
                    {
                        if (!updateCommandBuffer)
                            {
                                OPTICK_EVENT("Create Update Buffer");
                                updateCommandBuffer.create(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

                                VkCommandBufferBeginInfo info{};
                                info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                                info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                                vkBeginCommandBuffer(updateCommandBuffer, &info);
                            }

                        renderable.m_voxelSpace.updateBuffers(updateCommandBuffer);
                        hasUpdate = true;
                    }

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
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = &inheritanceInfo;

        VkCommandBuffer currentCommandBuffer = m_commandBuffers[m_frame];
        vkResetCommandBuffer(currentCommandBuffer, 0);
        if (vkBeginCommandBuffer(currentCommandBuffer, &beginInfo) != VK_SUCCESS)
            {
                // <error>
                return;
            }

        vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_surface.m_graphicsPipeline);

        for (auto &renderable : m_renderables)
            {
                VkDeviceSize offsets[] = { renderable.m_voxelSpace.getVertexMemoryOffset() };
                vulkanDescriptorSet *descriptorSet = renderable.m_descriptorSet->getDescriptorSet(m_frame);

                vkCmdBindIndexBuffer(currentCommandBuffer, renderable.m_voxelSpace.getBufferMemory().getUnderlyingBuffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, &renderable.m_voxelSpace.getBufferMemory().getUnderlyingBuffer(), offsets);
                
                vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_surface.m_pipelineLayout, 0, 1, &descriptorSet->getUnderlyingDescriptorSet(), 0, nullptr);
                vkCmdDrawIndexed(currentCommandBuffer, renderable.m_voxelSpace.getIndexCount(), 1, 0, 0, 0);
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
        m_inFlightFences[m_frame].wait();

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
                std::terminate();
                return;
            }

        if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
            {
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

        vkResetFences(m_device, 1, &m_inFlightFences[m_frame].getUnderlyingFence());
        if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_frame]) != VK_SUCCESS)
            {
                // <error>
                return;
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
            OPTICK_GPU_FLIP(m_swapChain);
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

glm::vec2 renderer::getSize() const
    {
        return { m_swapChain.getExtent().width, m_swapChain.getExtent().height };
    }

