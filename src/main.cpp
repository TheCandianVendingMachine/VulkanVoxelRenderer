// Roll-a-Ball
// A simple rolling ball game made with the sole purpose of figuring out how Vulkan and PhysX work on an implementation level
#include "typeDefines.hpp"
#include "graphics/window.hpp"
#include "graphics/renderer.hpp"
#include "taskGraph.hpp"
#include "clock.hpp"
#include <string>
#include <array>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include "graphics/vertex.hpp"
#include "graphics/vertexBuffer.hpp"
#include "graphics/vulkan/vulkanDescriptorSet.hpp"
#include "graphics/vulkan/vulkanSampler.hpp"
#include "graphics/image.hpp"
#include "graphics/uniformBuffer.hpp"
#include "graphics/indexBuffer.hpp"

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
        taskGraph taskGraph(1, 10);

        glfwInit();
        window app(1280, 720, "Roll-a-Ball");
        renderer renderer(app);

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

                while (accumulator >= updateRate)
                    {
                        accumulator -= updateRate;
                    }

                renderer.recordCommandBuffer();
                taskGraph.execute();

                renderer.display();
            }

        renderer.waitForDeviceIdle();

        taskGraph.stop();

        renderer.cleanup();
        app.cleanup();
        glfwTerminate();

        return 0;
    }

