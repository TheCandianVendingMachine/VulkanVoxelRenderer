// window.hpp
// creates and manages a GLFW window context. Handles all callbacks from events
#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class window
    {
        private:
            GLFWwindow *m_window = nullptr;

            unsigned int m_width = 0;
            unsigned int m_height = 0;

            bool m_cleanedUp = false;

        public:
            window() = default;
            window(unsigned int width, unsigned int height, const char *title);
            ~window();
            void cleanup();
            
            void createWindow(unsigned int width, unsigned int height, const char *title);
            void processEvents();

            bool isOpen() const;

            glm::vec2 getWindowSize() const;

            GLFWwindow *getUnderlyingWindow() const;

    };
