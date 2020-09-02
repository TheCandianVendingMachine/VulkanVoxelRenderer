// Voxels!
// Voxel rendering so I can hate myself
#include "typeDefines.hpp"
#include "graphics/window.hpp"
#include "graphics/renderer.hpp"
#include "taskGraph.hpp"
#include "clock.hpp"
#include "random.hpp"
#include <string>
#include <array>
#include <cstring>

#include "graphics/uniformBuffer.hpp"
#include "graphics/descriptorSet.hpp"
#include "graphics/descriptorSettings.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "voxel/voxelSpace.hpp"

#include "imgui.h"
#include <optick.h>

struct mvp
    {
        alignas(16) glm::mat4 m_model;
        alignas(16) glm::mat4 m_view;
        alignas(16) glm::mat4 m_projection;
    };

struct cameraUBO
    {
        glm::vec3 &m_origin;
        glm::vec3 &m_direction;
    };

int main()
    {
        fe::random rng;
        rng.startUp();
        #ifdef _DEBUG
        rng.seed(69420);
        #else
        rng.randomSeed();
        #endif

        // we install keyboard callbacks in ImGui creation. To use custom ones we must disable it and call the ImGui callbacks in the handler
        glfwInit();
        window app(1280, 720, "Voxels!!");

        descriptorSettings settings;
        settings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 1);
        
        renderer renderer(app, settings);
        renderer.initImGui(app);

        glm::vec3 cameraPos = { -23, -22, -2 };
        glm::vec3 cameraDir = glm::rotate(glm::vec3(1.f, 0.f, 0.f), glm::radians(45.f), glm::vec3(0.f, -1.f, 0.f));
        cameraDir = glm::rotate(cameraDir, glm::radians(45.f), glm::vec3(0.f, 0.f, 1.f));
        float m_cameraRotation = 0.f;

        mvp camera;
        camera.m_view = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0.f, -1.f, 0.f));
        camera.m_projection = glm::perspective(glm::radians(60.f), renderer.getSize().x / static_cast<float>(renderer.getSize().y), 0.1f, static_cast<float>(1 << 16));
        camera.m_projection[1][1] *= -1;

        constexpr float speed = 16.f;
        constexpr float rotationSpeed = 60.f;

        taskGraph taskGraph(2, 500);
        voxelSpace space;
        space.createWorld(&taskGraph);
        camera.m_model = space.getModelTransformation();

        uniformBuffer mvpUBO(camera);
        descriptorSet *voxelSpaceDescriptor = renderer.createDescriptorSet();
        voxelSpaceDescriptor->bindUBO(mvpUBO.getUniformBuffer(), mvpUBO.getBufferSize());

        descriptorSettings computeSettings;
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1);
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1);

        renderer.createComputePipeline(computeSettings, "shaders/raytracing.spv");

        cameraUBO cameraUBO{cameraPos, cameraDir};
        uniformBuffer viewUBO(cameraUBO);

        alignas(16) vulkanImage renderImage;
        vulkanImageView renderImageView;

        renderImage.create(256, 256, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        renderImageView.create(renderer.getDevice(), renderImage, VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        
        renderer.transitionImageLayout(renderImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        descriptorSet *computeDescriptor = renderer.createComputeDescriptorSet(0);
        computeDescriptor->bindImage(renderImageView);
        computeDescriptor->bindUBO(viewUBO.getUniformBuffer(), viewUBO.getBufferSize());
        
        fe::clock frameClock;
        fe::clock programClock;
        std::array<int64_t, 1000> frameTimes;
        unsigned int index = 0;

        const double updateRate = 1.0 / 60.0;
        fe::clock updateClock;
        double currentTime = updateClock.getTime().asSeconds();
        double newTime = 0.0;
        double frameTime = 0.0;

        double accumulator = 0.0;
        while (app.isOpen())
            {
                OPTICK_FRAME("MainThread");
                newTime = updateClock.getTime().asSeconds();
                frameTime = newTime - currentTime;

                if (frameTime > 5.0)
                    {
                        frameTime = updateRate;
                    }
                currentTime = newTime;
                accumulator += frameTime;

                frameTimes.at(index) = frameClock.getTime().asMicroseconds();
                frameClock.restart();

                app.processEvents();
                renderer.updateImGui();

                if (++index >= frameTimes.size())
                    {
                        int64_t avgTime = 0;
                        for (auto &frameTime : frameTimes)
                            {
                                avgTime += frameTime;
                            }

                        fe::time avgFrameTime = fe::microseconds(avgTime / frameTimes.size());
                        unsigned int fps = static_cast<unsigned int>(1.0 / avgFrameTime.asSeconds());

                        std::string formatted = std::to_string(avgFrameTime.asSeconds()) + " | " + std::to_string(fps);

                        glfwSetWindowTitle(app.getUnderlyingWindow(), formatted.c_str());
                        index = 0;
                    }

                bool mouseClick = false;
                int button = 0;
                bool keyPressed = false;
                while (accumulator >= updateRate)
                    {
                        OPTICK_EVENT("fixed timestep", Optick::Category::GameLogic);
                        accumulator -= updateRate;

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_W))
                            {
                                cameraPos += cameraDir * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_S))
                            {
                                cameraPos += -cameraDir * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_A))
                            {
                                cameraPos += -glm::cross(cameraDir, glm::vec3(0, -1.f, 0)) * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_D))
                            {
                                cameraPos += glm::cross(cameraDir, glm::vec3(0, -1.f, 0)) * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_Q))
                            {
                                cameraPos.y += -static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_Z))
                            {
                                cameraPos.y += static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_UP))
                            {
                                cameraDir = glm::rotate(cameraDir, glm::radians(rotationSpeed * (float)updateRate), glm::cross(cameraDir, glm::vec3(0, -1.f, 0)));
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_DOWN))
                            {
                                cameraDir = glm::rotate(cameraDir, -glm::radians(rotationSpeed * (float)updateRate), glm::cross(cameraDir, glm::vec3(0, -1.f, 0)));
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_LEFT))
                            {
                                cameraDir = glm::rotate(cameraDir, glm::radians(rotationSpeed * (float)updateRate), glm::vec3(0.f, -1.f, 0.f));
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_RIGHT))
                            {
                                cameraDir = glm::rotate(cameraDir, -glm::radians(rotationSpeed * (float)updateRate), glm::vec3(0.f, -1.f, 0.f));
                                keyPressed = true;
                            }

                        if (glfwGetMouseButton(app.getUnderlyingWindow(), GLFW_MOUSE_BUTTON_1))
                            {
                                mouseClick = true;
                                button |= 1;
                            }
                        if (glfwGetMouseButton(app.getUnderlyingWindow(), GLFW_MOUSE_BUTTON_2))
                            {
                                mouseClick = true;
                                button |= 2;
                            }
                    }
                
                if (keyPressed)
                    {
                        camera.m_view = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0, -1.f, 0));
                        mvpUBO.bind(camera);
                    }

                renderer.dispatchCompute(0, computeDescriptor, 8, 8, 1);

                if (mouseClick)
                    {
                        glm::ivec3 position = space.raycast(cameraPos, cameraDir);

                        if (button & 1)
                            {
                                space.setAt(position, voxelType::TEST_1);
                            }
                        else if (button & 2)
                            {
                                space.setAt(position, voxelType::NONE);
                            }
                    }

                //renderer.draw(*voxelSpaceDescriptor, space);

                renderer.preRecording();
                renderer.recordCommandBuffer();
                taskGraph.execute();

                renderer.display();
            }

        renderer.waitForDeviceIdle();

        renderer.deinitImGui();

        taskGraph.stop();

        viewUBO.destroy();

        renderImage.cleanup();
        renderImageView.cleanup();

        space.destroy();
        mvpUBO.destroy();

        renderer.cleanup();
        app.cleanup();
        glfwTerminate();

        return 0;
    }

