// renderer.hpp
// Renders objects onto screen
#pragma once
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "graphics/vulkan/vulkanAllocator.hpp"
#include "graphics/vulkan/vulkanInstance.hpp"
#include "graphics/vulkan/vulkanDevice.hpp"
#include "graphics/vulkan/vulkanPhysicalDevice.hpp"
#include "graphics/renderSurface.hpp"

#include "graphics/vertexBuffer.hpp"
#include "graphics/indexBuffer.hpp"

#include "graphics/vulkan/vulkanCommandPool.hpp"
#include "graphics/vulkan/vulkanCommandBuffer.hpp"
#include "graphics/vulkan/vulkanFence.hpp"
#include "graphics/vulkan/vulkanSemaphore.hpp"
#include "graphics/vulkan/vulkanDescriptorSet.hpp"

#include "graphics/descriptorSet.hpp"

class window;
class descriptorSettings;
class renderer
    {
        private:
            // todo: add a buffer for the command buffers so that each swapchain image owns the frame buffers
            static constexpr unsigned int c_maxFramesInFlight = 3;

            vulkanAllocator m_allocator;
            vulkanInstance m_instance;
            vulkanDevice m_device;
            vulkanPhysicalDevice m_physicalDevice;
            VkSurfaceKHR m_windowSurface = VK_NULL_HANDLE;

            renderSurface m_surface;

            VkQueue m_graphicsQueue = VK_NULL_HANDLE;
            VkQueue m_presentQueue = VK_NULL_HANDLE;

            vulkanCommandPool m_commandPool;
            std::vector<vulkanCommandBuffer> m_commandBuffers;
            std::vector<vulkanCommandBuffer> m_submissionCommandBuffers;

            std::vector<vulkanSemaphore> m_imageAvailableSemaphores;
            std::vector<vulkanSemaphore> m_renderFinishedSemaphores;
            std::vector<vulkanFence> m_inFlightFences;
            std::vector<VkFence> m_imagesInFlight;
            unsigned int m_frame = 0;

            struct renderable
                {
                    descriptorSet *m_descriptorSet = nullptr;
                    vertexBuffer *m_vertexBuffer = nullptr;
                    indexBuffer *m_indexBuffer = nullptr;
                };
            std::vector<renderable> m_renderables;

            void recordSubmissionCommandBuffer(VkCommandBuffer submissionBuffer);

        public:
            renderer(window &app, descriptorSettings &settings);
            ~renderer();

            void cleanup();

            void draw(descriptorSet &descriptorSet, vertexBuffer &vbo, indexBuffer &ibo);

            void preRecording();
            void recordCommandBuffer();

            void display();

            void waitForDeviceIdle();

            vulkanDevice &getDevice();
            const vulkanDevice &getDevice() const;

            descriptorSet *createDescriptorSet();
            glm::vec2 getSize() const;

    };
