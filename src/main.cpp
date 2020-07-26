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

struct quad
    {
        glm::vec3 m_position;
        char m_orientation = 0; // +-1 = xy, +-2 = xz, +-3 = yz
    };

void buildGeometry(const std::vector<quad> &quads, vertexBuffer &vbo, indexBuffer &ibo, const float size)
    {
        vertex vertexArr[4] = {};
        vertex &v0 = vertexArr[0];
        vertex &v1 = vertexArr[1];
        vertex &v2 = vertexArr[2];
        vertex &v3 = vertexArr[3];

        fe::index indexArrPositive[6] = {
            0, 1, 2, 0, 2, 3
        };
        fe::index indexArrNegative[6] = {
            2, 1, 0, 3, 2, 0
        };

        glm::vec3 colours[] = {
            { 0.23f, 0.48f, 0.34f },
            { 0.53f, 0.81f, 0.92f },
            { 0.37f, 0.50f, 0.22f }
        };

        for (auto &quad : quads)
            {
                v0.m_colour = colours[std::abs(quad.m_orientation) - 1];
                v1.m_colour = colours[std::abs(quad.m_orientation) - 1];
                v2.m_colour = colours[std::abs(quad.m_orientation) - 1];
                v3.m_colour = colours[std::abs(quad.m_orientation) - 1];

                switch (std::abs(quad.m_orientation))
                    {
                        case 1:
                            v0.m_position.x = quad.m_position.x;
                            v0.m_position.y = quad.m_position.y;
                            v0.m_position.z = quad.m_position.z;

                            v1.m_position.x = quad.m_position.x + size;
                            v1.m_position.y = quad.m_position.y;
                            v1.m_position.z = quad.m_position.z;

                            v2.m_position.x = quad.m_position.x + size;
                            v2.m_position.y = quad.m_position.y + size;
                            v2.m_position.z = quad.m_position.z;

                            v3.m_position.x = quad.m_position.x;
                            v3.m_position.y = quad.m_position.y + size;
                            v3.m_position.z = quad.m_position.z;
                            break;
                        case 2:
                            v0.m_position.x = quad.m_position.x;
                            v0.m_position.y = quad.m_position.z;
                            v0.m_position.z = quad.m_position.y;

                            v1.m_position.x = quad.m_position.x + size;
                            v1.m_position.y = quad.m_position.z;
                            v1.m_position.z = quad.m_position.y;

                            v2.m_position.x = quad.m_position.x + size;
                            v2.m_position.y = quad.m_position.z;
                            v2.m_position.z = quad.m_position.y + size;

                            v3.m_position.x = quad.m_position.x;
                            v3.m_position.y = quad.m_position.z;
                            v3.m_position.z = quad.m_position.y + size;
                            break;
                        case 3:
                            v0.m_position.x = quad.m_position.z;
                            v0.m_position.y = quad.m_position.x;
                            v0.m_position.z = quad.m_position.y;

                            v1.m_position.x = quad.m_position.z;
                            v1.m_position.y = quad.m_position.x + size;
                            v1.m_position.z = quad.m_position.y;

                            v2.m_position.x = quad.m_position.z;
                            v2.m_position.y = quad.m_position.x + size;
                            v2.m_position.z = quad.m_position.y + size;

                            v3.m_position.x = quad.m_position.z;
                            v3.m_position.y = quad.m_position.x;
                            v3.m_position.z = quad.m_position.y + size;
                            break;
                        default:
                            break;
                    }

                vbo.addVertices(vertexArr, 4);
                if (quad.m_orientation < 0)
                    {
                        ibo.addIndices(indexArrNegative, 6);
                    }
                else 
                    {
                        ibo.addIndices(indexArrPositive, 6);
                    }
                
                indexArrPositive[0] += 4;
                indexArrPositive[1] += 4;
                indexArrPositive[2] += 4;
                indexArrPositive[3] += 4;
                indexArrPositive[4] += 4;
                indexArrPositive[5] += 4;

                indexArrNegative[0] += 4;
                indexArrNegative[1] += 4;
                indexArrNegative[2] += 4;
                indexArrNegative[3] += 4;
                indexArrNegative[4] += 4;
                indexArrNegative[5] += 4;
            }
    }

int main()
    {
        glfwInit();
        window app(1280, 720, "Voxels!!");

        renderer renderer(app);

        constexpr int size = 16;
        constexpr float voxelSize = 1.0f;

        char voxelArray[size][size][size];
        std::memset(voxelArray, 1, size * size * size);

        vertexBuffer vbo(0, false);
        indexBuffer ibo(0, false);
        std::vector<quad> quadVector;
        for (int x = 0; x < size; x++)
            {
                for (int y = 0; y < size; y++)
                    {
                        for (int z = 0; z < size; z++)
                            {
                                if (voxelArray[x][y][z] != 0)
                                    {
                                        quad quads[6] = {};
                                        quads[0].m_orientation = 1;
                                        quads[0].m_position.x = x;
                                        quads[0].m_position.y = y;
                                        quads[0].m_position.z = z + voxelSize;
                                        quads[1].m_orientation = -1;
                                        quads[1].m_position.x = x;
                                        quads[1].m_position.y = y;

                                        quads[2].m_orientation = 2;
                                        quads[2].m_position.x = x;
                                        quads[2].m_position.y = z;
                                        quads[3].m_orientation = -2;
                                        quads[3].m_position.x = x;
                                        quads[3].m_position.y = z;
                                        quads[3].m_position.z = y + voxelSize;

                                        quads[4].m_orientation = 3;
                                        quads[4].m_position.x = y;
                                        quads[4].m_position.y = z;
                                        quads[4].m_position.z = x + voxelSize;
                                        quads[5].m_orientation = -3;
                                        quads[5].m_position.x = y;
                                        quads[5].m_position.y = z;

                                        quadVector.push_back(quads[0]);
                                        quadVector.push_back(quads[1]);
                                        quadVector.push_back(quads[2]);
                                        quadVector.push_back(quads[3]);
                                        quadVector.push_back(quads[4]);
                                        quadVector.push_back(quads[5]);
                                    }
                            }
                    }
            }

        buildGeometry(quadVector, vbo, ibo, voxelSize);

        uniformBuffer mvpUBO;
        uniformBuffer transformUBO;

        transformation cameraPos;
        cameraPos.m_position = { 3, -1, 0 };
        float m_cameraRotation = 45.f;

        mvp camera;
        camera.m_model = glm::rotate(glm::mat4(1.f), glm::radians(m_cameraRotation), glm::vec3(0.f, 1.f, 0.f));
        camera.m_view = glm::lookAt(cameraPos.m_position, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
        camera.m_projection = glm::perspective(glm::radians(60.f), renderer.getSize().x / static_cast<float>(renderer.getSize().y), 0.1f, 100.f);
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

