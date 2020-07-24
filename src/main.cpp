// Voxels!
// Voxel rendering so I can hate myself
#include "typeDefines.hpp"
#include "graphics/window.hpp"
#include "graphics/renderer.hpp"
#include "taskGraph.hpp"
#include "clock.hpp"
#include <string>
#include <array>
#include <cstring>

#include "graphics/vertexBuffer.hpp"
#include "graphics/indexBuffer.hpp"
#include "graphics/uniformBuffer.hpp"
#include "graphics/vertex.hpp"
#include "graphics/descriptorSet.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

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
        glfwInit();
        window app(1280, 720, "Voxels!!");

        renderer renderer(app);

        constexpr int size = 1;
        constexpr float voxelSize = 0.2f;

        char voxelArray[size][size][size];
        std::memset(voxelArray, 1, size * size * size);

        std::vector<vertex> vertices;
        std::vector<fe::index> indices;

        for (int x = 0; x < size; x++)
            {
                for (int y = 0; y < size; y++)
                    {
                        for (int z = 0; z < size; z++)
                            {
                                if (voxelArray[x][y][z] != 0)
                                    {
                                        vertex v;

                                        unsigned int indexBegin = vertices.size();

                                        float xPos = static_cast<float>(x) * voxelSize;
                                        float yPos = static_cast<float>(y) * voxelSize;
                                        float zPos = static_cast<float>(z) * voxelSize;

                                        v.m_colour = (glm::vec3{ 240.f, 163.f, 255.f }) / 255.f;
                                        v.m_position.x = xPos;
                                        v.m_position.y = yPos;
                                        v.m_position.z = zPos;
                                        vertices.push_back(v);

                                        v.m_colour = (glm::vec3{ 0.f, 117.f, 220.f }) / 255.f;
                                        v.m_position.x = xPos + voxelSize;
                                        v.m_position.y = yPos;
                                        v.m_position.z = zPos;
                                        vertices.push_back(v);

                                        v.m_colour = (glm::vec3{ 153.f, 63.f, 0.f }) / 255.f;
                                        v.m_position.x = xPos + voxelSize;
                                        v.m_position.y = yPos;
                                        v.m_position.z = zPos + voxelSize;
                                        vertices.push_back(v);

                                        v.m_colour = (glm::vec3{ 76.f, 0.f, 92.f }) / 255.f;
                                        v.m_position.x = xPos;
                                        v.m_position.y = yPos;
                                        v.m_position.z = zPos + voxelSize;
                                        vertices.push_back(v);
                                        ///////////////////////////
                                        v.m_colour = (glm::vec3{ 25.f, 25.f, 25.f }) / 255.f;
                                        v.m_position.x = xPos;
                                        v.m_position.y = yPos + voxelSize;
                                        v.m_position.z = zPos;
                                        vertices.push_back(v);

                                        v.m_colour = (glm::vec3{ 0.f, 92.f, 49.f }) / 255.f;
                                        v.m_position.x = xPos + voxelSize;
                                        v.m_position.y = yPos + voxelSize;
                                        v.m_position.z = zPos;
                                        vertices.push_back(v);

                                        v.m_colour = (glm::vec3{ 43.f, 206.f, 72.f }) / 255.f;
                                        v.m_position.x = xPos + voxelSize;
                                        v.m_position.y = yPos + voxelSize;
                                        v.m_position.z = zPos + voxelSize;
                                        vertices.push_back(v);

                                        v.m_colour = (glm::vec3{ 255.f, 204.f, 153.f }) / 255.f;
                                        v.m_position.x = xPos;
                                        v.m_position.y = yPos + voxelSize;
                                        v.m_position.z = zPos + voxelSize;
                                        vertices.push_back(v);

                                        indices.insert(indices.end(), {
                                            indexBegin + 0, indexBegin + 1, indexBegin + 2, indexBegin + 0, indexBegin + 2, indexBegin + 3,
                                            indexBegin + 0, indexBegin + 3, indexBegin + 7, indexBegin + 0, indexBegin + 7, indexBegin + 4,
                                            indexBegin + 0, indexBegin + 1, indexBegin + 5, indexBegin + 0, indexBegin + 5, indexBegin + 4,

                                            indexBegin + 6, indexBegin + 3, indexBegin + 2, indexBegin + 6, indexBegin + 7, indexBegin + 3,
                                            indexBegin + 6, indexBegin + 4, indexBegin + 7, indexBegin + 6, indexBegin + 5, indexBegin + 4,
                                            indexBegin + 6, indexBegin + 1, indexBegin + 5, indexBegin + 6, indexBegin + 2, indexBegin + 1,
                                        });
                                    }
                            }
                    }
            }

        vertexBuffer vbo(vertices.size(), false);
        indexBuffer ibo(indices.size(), false);
        uniformBuffer mvpUBO;
        uniformBuffer transformUBO;

        transformation cameraPos;
        cameraPos.m_position = { 3, -1, 0 };

        mvp camera;
        camera.m_model = glm::rotate(glm::mat4(1.f), 5 * glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
        camera.m_view = glm::lookAt(cameraPos.m_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
        camera.m_projection = glm::perspective(glm::radians(60.f), renderer.getSize().x / static_cast<float>(renderer.getSize().y), 0.1f, 10.f);
        camera.m_projection[1][1] *= -1;


        constexpr float speed = 5.f;
        transformation t;
        t.m_position = {0, 0, 0};

        mvpUBO.create(camera);
        transformUBO.create(t);

        descriptorSet *ds = renderer.createDescriptorSet();
        ds->bindUBO(mvpUBO.getUniformBuffer(), mvpUBO.getBufferSize());
        ds->bindUBO(transformUBO.getUniformBuffer(), transformUBO.getBufferSize());
        ds->update();

        vbo.addVertices(vertices.data(), vertices.size());
        ibo.addIndices(indices.data(), indices.size());

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
                while (accumulator >= updateRate)
                    {
                        accumulator -= updateRate;

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_W))
                            {
                                cameraPos.m_position.x += -speed * updateRate;
                                keyPressed = true;
                            }

                        if (glfwGetKey(app.getUnderlyingWindow(), GLFW_KEY_S))
                            {
                                cameraPos.m_position.x += speed * updateRate;
                                keyPressed = true;
                            }
                    }

                if (keyPressed)
                    {
                        camera.m_view = glm::lookAt(cameraPos.m_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
                    }

                camera.m_model = glm::rotate(glm::mat4(1.f), (float)currentTime * glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
                mvpUBO.bind(camera);

                renderer.draw(*ds, vbo, &ibo);

                renderer.recordCommandBuffer();
                taskGraph.execute();

                renderer.display();
            }

        renderer.waitForDeviceIdle();

        taskGraph.stop();

        vbo.destroy();
        ibo.destroy();
        mvpUBO.destroy();
        transformUBO.destroy();

        renderer.cleanup();
        app.cleanup();
        glfwTerminate();

        return 0;
    }

