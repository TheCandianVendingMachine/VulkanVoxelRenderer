// Voxels!
// Voxel rendering so I can hate myself
#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#undef TINYOBJLOADER_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#undef STB_IMAGE_IMPLEMENTATION

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

#include "voxel/sparseVoxelOctree.hpp"
#include "voxel/voxelGrid.hpp"
#include "voxel/heightmap.hpp"
#include "voxel/raytracer.hpp"
#include "graphics/noise.hpp"

struct mvp
    {
        alignas(16) glm::mat4 m_model;
        alignas(16) glm::mat4 m_view;
        alignas(16) glm::mat4 m_projection;
    };

struct cameraUBO
    {
        alignas(16) glm::mat4 m_cameraToWorld;
        alignas(16) glm::mat4 m_inverseProjection;
    };

struct light
    {
        alignas(16) glm::vec3 m_direction = { 0.f, 0.f, 0.f };
        alignas(16) float m_intensity = 1.f;
    };

void loadTexture(const char *file, vulkanImage &image, renderer &renderer)
    {
        int channels;
        int width;
        int height;
        stbi_uc *m_pixelData = stbi_load(file, &width, &height, &channels, STBI_rgb_alpha);

        image.create(width, height, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_TYPE_2D);

        unsigned int imageSize = width * height * 4;
        vulkanBuffer buffer;
        buffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        void *data;
        vmaMapMemory(*globals::g_vulkanAllocator, buffer.getUnderlyingAllocation(), &data);
        std::memcpy(data, m_pixelData, static_cast<size_t>(imageSize));
        vmaUnmapMemory(*globals::g_vulkanAllocator, buffer.getUnderlyingAllocation());            

        stbi_image_free(m_pixelData);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            (uint32_t)width,
            (uint32_t)height,
            1
        };

        renderer.transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        renderer.copyBufferToImage(buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        renderer.transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        buffer.cleanup();
    }

int main()
    {
        fe::random rng;
        rng.startUp();
        #ifdef _DEBUG
        rng.seed(69420);
        #else
        rng.randomSeed();
        #endif

        constexpr int size = 128;
        constexpr int depth = 10;
        glm::vec3 rgb(222, 215, 252);
        rgb /= 255.f;

        sparseVoxelOctree testSVO({size, size, size});
        /*testSVO.load("TestVoxelTree1.txt");
        //testSVO.addVoxel(1.0, 1.0, 1.0, depth, rgb.r, rgb.g, rgb.b);
        //testSVO.addVoxel(120.0, 1.0, 1.0, depth, rgb.r, rgb.g, rgb.b);
        //testSVO.addVoxel(1.0, 100.0, 1.0, depth, rgb.r, rgb.g, rgb.b);
        testSVO.addVoxel(1.0, 1.0, 100.0, depth, rgb.r, rgb.g, rgb.b);
        
        //testSVO.addVoxel(70.0, 100.0, 1.0, depth, rgb.r, rgb.g, rgb.b);
        //testSVO.addVoxel(0.0, 100.0, 100.0, depth, rgb.r, rgb.g, rgb.b);
        //testSVO.addVoxel(100.0, 1.0, 100.0, depth, rgb.r, rgb.g, rgb.b);
        testSVO.addVoxel(100.0, 100.0, 100.0, depth, rgb.r, rgb.g, rgb.b);

        testSVO.save("TestVoxelTree1.txt");*/

        // we install keyboard callbacks in ImGui creation. To use custom ones we must disable it and call the ImGui callbacks in the handler
        glfwInit();
        window app(1920, 1080, "Voxels!!");

        descriptorSettings settings;
        settings.addSetting(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, 1);
        
        renderer renderer(app, settings);
        renderer.initImGui(app);

        voxelGrid testGrid({ 128, 128, 128 }, renderer);
        //testGrid.load("testVoxelGrid.txt");

        for (int i = 0; i < 5000; i++)
            {
                testGrid.add({ rng.generate(0, 127), rng.generate(0, 127), rng.generate(0, 127) }, voxel{ rgb.r, rgb.g, rgb.b });
            }

        testGrid.init(renderer);
        testGrid.bake();
        testGrid.bakeImage(renderer);
        testGrid.save("testVoxelGrid.txt");

        glm::vec3 cameraPos = { 1509.f, 757.f, 1760.f };
        glm::vec3 cameraDir = { 0.300850391, -0.278304756, -0.912167728 };
        float m_cameraRotation = 0.f;

        mvp mvpCamera;
        mvpCamera.m_view = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0.f, -1.f, 0.f));
        mvpCamera.m_projection = glm::perspective(glm::radians(100.f), renderer.getSize().x / static_cast<float>(renderer.getSize().y), 0.1f, static_cast<float>(1 << 16));
        mvpCamera.m_projection[1][1] *= -1;

        float speed = 250.f;
        constexpr float rotationSpeed = 50.f;

        taskGraph taskGraph(2, 500);
        voxelSpace space;
        space.createWorld(&taskGraph);
        mvpCamera.m_model = space.getModelTransformation();

        glm::mat4 translation = glm::translate(glm::mat4(1.f), cameraPos);
        glm::quat quat = glm::angleAxis(glm::radians(-30.f), glm::normalize(glm::vec3{ 1.f, 0.f, 0.f }));

        cameraUBO camera;
        camera.m_cameraToWorld = translation * glm::toMat4(quat);
        camera.m_inverseProjection = glm::inverse(mvpCamera.m_projection);

        uniformBuffer viewUBO(camera);

        //storageBuffer sphereBuffer(spheres.size(), sizeof(sphere), spheres.data());

        light light;
        light.m_direction = glm::normalize(glm::vec3(0.709, -0.670, -0.231));
        light.m_intensity = 1.f;

        uniformBuffer lightUBO(light);

        heightmap hm("banff_heightmap/banff_heightmap Height Map (ASTER 30m).png", renderer);
        //heightmap hm("banff_heightmap/test.jpg", renderer);

        //storageBuffer voxelStorageBuffer;
        //testSVO.mapToStorageBuffer(voxelStorageBuffer);

        raytracer raytracer(renderer, hm, {1920, 1080}, "textures/skybox.jpg", viewUBO, lightUBO, testGrid);
        raytracer.addGroundTexture("textures/TexturesCom_Grass0202_1_seamless_S.jpg", 20.f, 200.f);
        raytracer.addGroundTexture("textures/TexturesCom_Grass0157_1_seamless_S.jpg", 300.f, 500.f);
        raytracer.addGroundTexture("textures/TexturesCom_RockRough0030_2_seamless_S.jpg", 600.f, 1600.f);
        raytracer.addGroundTexture("textures/TexturesCom_SoilMud0110_1_seamless_S.jpg", -20.f, -200.f);

        fe::clock frameClock;
        fe::clock programClock;
        std::array<int64_t, 100> frameTimes = {};
        unsigned int index = 0;

        const double updateRate = 1.0 / 60.0;
        fe::clock updateClock;
        double currentTime = updateClock.getTime().asSeconds();
        double newTime = 0.0;
        double frameTime = 0.0;

        fe::clock fpsUpdateClock;

        double accumulator = 0.0;
        while (app.isOpen())
            {
                OPTICK_FRAME("MainThread");
                newTime = updateClock.getTime().asSeconds();
                frameTime = newTime - currentTime;
                testGrid.rayIntersects(cameraPos, cameraDir);

                if (frameTime > 5.0)
                    {
                        frameTime = updateRate;
                    }
                currentTime = newTime;
                accumulator += frameTime;

                frameTimes[index] = frameClock.getTime().asMicroseconds();
                frameClock.restart();

                app.processEvents();
                renderer.updateImGui();

                if (++index >= frameTimes.size() || fpsUpdateClock.getTime() >= fe::seconds(1))
                    {
                        int64_t avgTime = 0;
                        for (auto &frameTime : frameTimes)
                            {
                                avgTime += frameTime;
                                frameTime = 0;
                            }

                        fe::time avgFrameTime = fe::microseconds(avgTime / index);
                        unsigned int fps = static_cast<unsigned int>(1.0 / avgFrameTime.asSeconds());

                        std::string formatted = std::to_string(avgFrameTime.asSeconds()) + " | " + std::to_string(fps) + " | " + std::to_string(fpsUpdateClock.getTime().asSeconds());

                        glfwSetWindowTitle(app.getUnderlyingWindow(), formatted.c_str());
                        index = 0;
                        fpsUpdateClock.restart();
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
                                cameraPos += -glm::cross(cameraDir, glm::vec3(0, 1.f, 0)) * static_cast<float>(speed * updateRate);
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_D))
                            {
                                cameraPos += glm::cross(cameraDir, glm::vec3(0, 1.f, 0)) * static_cast<float>(speed * updateRate);
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
                                cameraDir = glm::rotate(cameraDir, glm::radians(rotationSpeed * (float)updateRate), glm::cross(cameraDir, glm::vec3(0, 1.f, 0)));
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_DOWN))
                            {
                                cameraDir = glm::rotate(cameraDir, -glm::radians(rotationSpeed * (float)updateRate), glm::cross(cameraDir, glm::vec3(0, 1.f, 0)));
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_LEFT))
                            {
                                cameraDir = glm::rotate(cameraDir, glm::radians(rotationSpeed * (float)updateRate), glm::vec3(0.f, 1.f, 0.f));
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_RIGHT))
                            {
                                cameraDir = glm::rotate(cameraDir, -glm::radians(rotationSpeed * (float)updateRate), glm::vec3(0.f, 1.f, 0.f));
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

                ImGui::Begin("Camera");
                bool slid = false;
                slid |= ImGui::SliderFloat("Light Direction X", &light.m_direction.x, -1.f, 1.f);
                slid |= ImGui::SliderFloat("Light Direction Y", &light.m_direction.y, -1.f, 1.f);
                slid |= ImGui::SliderFloat("Light Direction Z", &light.m_direction.z, -1.f, 1.f);
                ImGui::SliderFloat("Speed", &speed, 0.f, 5000.f);

                float posX = cameraPos.x;
                float posZ = cameraPos.z;
                float posY = cameraPos.y;
                bool posChange = ImGui::SliderFloat("PosX", &posX, 0.f, 500.f);
                posChange |= ImGui::SliderFloat("PosY", &posY, 0.f, 500.f);
                posChange |= ImGui::SliderFloat("PosZ", &posZ, 0.f, 500.f);
                cameraPos.y = posY;
                cameraPos.x = posX;
                cameraPos.z = posZ;

                ImGui::Text("%f", hm.getHeight(cameraPos));
                
                light.m_direction = glm::normalize(light.m_direction);

                //cameraPos.y = hm.getHeight(cameraPos) + 1.83f; posChange = true;

                ImGui::End();

                if (slid)
                    {
                        lightUBO.bind(light);
                    }
                
                if (keyPressed || posChange)
                    {
                        //camera.m_view = ;
                        //mvpUBO.bind(camera);

                        quat = glm::quatLookAt(cameraDir, glm::vec3(0.f, 1.f, 0.f));

                        translation = glm::translate(glm::mat4(1.f), cameraPos);
                        camera.m_cameraToWorld = translation * glm::toMat4(quat);
                        camera.m_inverseProjection = glm::inverse(mvpCamera.m_projection);

                        viewUBO.bind(camera);
                    }
                
                /*if (mouseClick)
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
                    }*/

                raytracer.dispatch();
                raytracer.draw(true);

                renderer.preRecording();
                renderer.recordCommandBuffer();
                taskGraph.execute();

                renderer.display();
            }

        renderer.waitForDeviceIdle();

        renderer.deinitImGui();

        taskGraph.stop();

        //voxelStorageBuffer.destroy();
        //sphereBuffer.destroy();
        viewUBO.destroy();
        lightUBO.destroy();

        raytracer.destroy();

        testGrid.destroy();
        hm.destroy();

        space.destroy();
        //mvpUBO.destroy();

        renderer.cleanup();
        app.cleanup();
        glfwTerminate();

        return 0;
    }

