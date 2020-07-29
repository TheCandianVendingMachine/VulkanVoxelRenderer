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
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "voxel/voxelSpace.hpp"

struct mvp
    {
        alignas(16) glm::mat4 m_model;
        alignas(16) glm::mat4 m_view;
        alignas(16) glm::mat4 m_projection;
    };

struct transformation
    {
        alignas(16) glm::vec3 m_position;
    };

int main()
    {
        fe::random rng;
        rng.startUp();
        #ifdef _DEBUG
        rng.seed(69420);
        #endif

        voxelSpace space;

        glfwInit();
        window app(1280, 720, "Voxels!!");

        renderer renderer(app);

        uniformBuffer mvpUBO;
        uniformBuffer transformUBO;

        transformation cameraPos;
        cameraPos.m_position = { 3, -1, 0 };
        float m_cameraRotation = 45.f;

        mvp camera;
        camera.m_model = glm::rotate(glm::mat4(1.f), glm::radians(m_cameraRotation), glm::vec3(0.f, 1.f, 0.f));
        camera.m_view = glm::lookAt(cameraPos.m_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
        camera.m_projection = glm::perspective(glm::radians(60.f), renderer.getSize().x / static_cast<float>(renderer.getSize().y), 0.1f, static_cast<float>(1 << 16));
        camera.m_projection[1][1] *= -1;

        constexpr float speed = 10.f;
        constexpr float rotationSpeed = 90.f;
        transformation t;
        t.m_position = {0, 0, 0};

        mvpUBO.create(camera);
        transformUBO.create(t);

        descriptorSet *ds = renderer.createDescriptorSet();
        ds->bindUBO(mvpUBO.getUniformBuffer(), mvpUBO.getBufferSize());
        ds->bindUBO(transformUBO.getUniformBuffer(), transformUBO.getBufferSize());
        ds->update();

        taskGraph taskGraph(1, 10);

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

                bool keyPressed = false;
                glm::vec3 translation = { 0, 0, 0 };
                while (accumulator >= updateRate)
                    {
                        accumulator -= updateRate;

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_W))
                            {
                                translation.x += speed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_S))
                            {
                                translation.x += -speed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_A))
                            {
                                translation.z += speed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_D))
                            {
                                translation.z += -speed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_Q))
                            {
                                translation.y += speed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_Z))
                            {
                                translation.y += -speed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_LEFT))
                            {
                                m_cameraRotation += -rotationSpeed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_RIGHT))
                            {
                                m_cameraRotation += rotationSpeed * updateRate;
                                keyPressed = true;
                            }
                    }

                if (keyPressed)
                    {
                        camera.m_model = glm::rotate(glm::mat4(1.f), glm::radians(m_cameraRotation), glm::vec3(0.f, 1.f, 0.f));
                        camera.m_view = glm::translate(camera.m_view, translation);
                        mvpUBO.bind(camera);
                    }

                renderer.draw(*ds, space.getVertexBuffer(), &space.getIndexBuffer());

                renderer.recordCommandBuffer();
                taskGraph.execute();

                renderer.display();
            }

        renderer.waitForDeviceIdle();

        taskGraph.stop();

        space.destroy();
        mvpUBO.destroy();
        transformUBO.destroy();

        renderer.cleanup();
        app.cleanup();
        glfwTerminate();

        return 0;
    }

