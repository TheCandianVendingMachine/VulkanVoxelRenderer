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
#include <vector>

#include "graphics/uniformBuffer.hpp"
#include "graphics/descriptorSet.hpp"
#include "graphics/descriptorSettings.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "voxel/voxelSpace.hpp"

#include "graphics/storageBuffer.hpp"

#include "graphics/vulkan/vulkanSampler.hpp"

#include "imgui.h"
#include <optick.h>

#include <glm/gtx/quaternion.hpp>

struct mvp
    {
        alignas(16) glm::mat4 m_model;
        alignas(16) glm::mat4 m_view;
        alignas(16) glm::mat4 m_projection;
    };

struct cameraUBO
    {
        glm::mat4 m_cameraToWorld;
        glm::mat4 m_inverseProjection;
    };

struct light
    {
        glm::vec3 m_direction = { 0.f, 0.f, 0.f };
        float m_intensity = 1.f;
    };

struct sphere
    {
        glm::vec3 m_position;
        glm::vec3 m_albedo;
        glm::vec3 m_specular;
        float m_radius = 1.f;
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
        settings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, 1);
        
        renderer renderer(app, settings);
        renderer.initImGui(app);

        glm::vec3 cameraPos = { 0, 5.f, 5.f };
        glm::vec3 cameraDir = glm::rotate(glm::vec3(1.f, 0.f, 0.f), glm::radians(0.f), glm::vec3(0.f, -1.f, 0.f));
        cameraDir = glm::rotate(cameraDir, glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f));
        float m_cameraRotation = 0.f;

        mvp mvpCamera;
        mvpCamera.m_view = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0.f, -1.f, 0.f));
        mvpCamera.m_projection = glm::perspective(glm::radians(100.f), renderer.getSize().x / static_cast<float>(renderer.getSize().y), 0.1f, static_cast<float>(1 << 16));
        mvpCamera.m_projection[1][1] *= -1;

        constexpr float speed = 16.f;
        constexpr float rotationSpeed = 60.f;

        taskGraph taskGraph(2, 500);
        voxelSpace space;
        space.createWorld(&taskGraph);
        mvpCamera.m_model = space.getModelTransformation();

        descriptorSettings computeSettings;
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1);
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1);
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1);
        computeSettings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT, 1);
        
        renderer.createComputePipeline(computeSettings, "shaders/voxel_raytracing.spv");

        glm::mat4 translation = glm::translate(glm::mat4(1.f), cameraPos);
        glm::quat quat = glm::angleAxis(glm::radians(-30.f), glm::normalize(glm::vec3{ 1.f, 0.f, 0.f }));

        cameraUBO camera;
        camera.m_cameraToWorld = translation * glm::toMat4(quat);
        camera.m_inverseProjection = glm::inverse(mvpCamera.m_projection);

        uniformBuffer viewUBO(camera);

        alignas(16) vulkanImage renderImage;
        vulkanImageView renderImageView;

        glm::ivec2 imageSize{2560, 1440};
        renderImage.create(imageSize.x, imageSize.y, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        renderImageView.create(renderer.getDevice(), renderImage, VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        
        renderer.transitionImageLayout(renderImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        std::vector<sphere> spheres;
        glm::vec3 spherePos = {0, 1.03f, 0};
        for (int x = 0; x < 10; x++)
            {
                for (int y = 0; y < 10; y++)
                    {
                        glm::vec3 colour = { rng.generate(0.f, 1.f), rng.generate(0.f, 1.f), rng.generate(0.f, 1.f) };
                        float rngValue = rng.generate(0.f, 1.f);
                        bool metal = rngValue < 0.5f;

                        sphere s;
                        s.m_radius = 1.f;
                        s.m_albedo = metal ? glm::vec3(0.f) : colour;
                        s.m_specular = metal ? colour : glm::vec3(1.f) * 0.04f;

                        s.m_position = spherePos;
                        spherePos.z += 4.f;

                        spheres.push_back(s);
                    }
                spherePos.z = 0.f;
                spherePos.x += 4.f;
            }

        storageBuffer sphereBuffer(spheres.size(), sizeof(sphere), spheres.data());

        light light;
        light.m_direction = glm::normalize(glm::vec3(-2.f, -10.f, 5.f));
        light.m_intensity = 1.f;

        uniformBuffer lightUBO(light);

        descriptorSet *computeDescriptor = renderer.createComputeDescriptorSet(0);
        computeDescriptor->bindImage(renderImageView);
        computeDescriptor->bindUBO(viewUBO.getUniformBuffer(), viewUBO.getBufferSize());
        computeDescriptor->bindUBO(lightUBO.getUniformBuffer(), lightUBO.getBufferSize());
        computeDescriptor->bindSBO(sphereBuffer.getStorageBuffer(), sphereBuffer.getBufferSize());
        computeDescriptor->update();
        
        vulkanImage finalImage(imageSize.x, imageSize.y, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        vulkanImageView finalImageView(renderer.getDevice(), finalImage, VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        vulkanSampler finalImageSampler(renderer.getDevice(), finalImage.mipLevels);

        renderer.transitionImageLayout(finalImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        descriptorSet *voxelSpaceDescriptor = renderer.createDescriptorSet();
        voxelSpaceDescriptor->bindImage(finalImageView, finalImageSampler);

        fe::clock frameClock;
        fe::clock programClock;
        std::array<int64_t, 100> frameTimes;
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

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_D))
                            {
                                cameraPos += cameraDir * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_A))
                            {
                                cameraPos += -cameraDir * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_S))
                            {
                                cameraPos += -glm::cross(cameraDir, glm::vec3(0, -1.f, 0)) * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_W))
                            {
                                cameraPos += glm::cross(cameraDir, glm::vec3(0, -1.f, 0)) * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_Z))
                            {
                                cameraPos.y += -static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_Q))
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

                ImGui::Begin("Raycast Settings");
                bool slid = false;
                slid |= ImGui::SliderFloat("Light Direction X", &light.m_direction.x, -1.f, 1.f);
                slid |= ImGui::SliderFloat("Light Direction Y", &light.m_direction.y, -1.f, 1.f);
                slid |= ImGui::SliderFloat("Light Direction Z", &light.m_direction.z, -1.f, 1.f);
                ImGui::End();

                if (slid)
                    {
                        lightUBO.bind(light);
                    }
                
                if (keyPressed)
                    {
                        //camera.m_view = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0, -1.f, 0));
                        //mvpUBO.bind(camera);
                        translation = glm::translate(glm::mat4(1.f), cameraPos);
                        camera.m_cameraToWorld = translation * glm::toMat4(quat);
                        camera.m_inverseProjection = glm::inverse(mvpCamera.m_projection);

                        viewUBO.bind(camera);
                    }

                renderer.dispatchCompute(0, computeDescriptor, imageSize.x / 8, imageSize.y / 8, 1);

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

                renderer.blitImage(renderImage, VK_IMAGE_LAYOUT_GENERAL, finalImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, imageSize.x, imageSize.y);

                renderer.draw(*voxelSpaceDescriptor);

                renderer.preRecording();
                renderer.recordCommandBuffer();
                taskGraph.execute();

                renderer.display();//break;
            }

        renderer.waitForDeviceIdle();

        renderer.deinitImGui();

        taskGraph.stop();

        sphereBuffer.destroy();
        viewUBO.destroy();
        lightUBO.destroy();

        finalImage.cleanup();
        finalImageSampler.cleanup();
        finalImageView.cleanup();

        renderImage.cleanup();
        renderImageView.cleanup();

        space.destroy();
        //mvpUBO.destroy();

        renderer.cleanup();
        app.cleanup();
        glfwTerminate();

        return 0;
    }

