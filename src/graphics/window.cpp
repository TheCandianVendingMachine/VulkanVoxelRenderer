#include "graphics/window.hpp"

window::window(unsigned int width, unsigned int height, const char *title)
    {
        createWindow(width, height, title);
    }

window::~window()
    {
        cleanup();
    }

void window::cleanup()
    {
        if (m_cleanedUp) { return; }

        glfwDestroyWindow(m_window);
        m_window = nullptr;
        m_cleanedUp = true;
    }

void window::createWindow(unsigned int width, unsigned int height, const char *title)
    {
        m_width = width;
        m_height = height;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        m_cleanedUp = false;
    }

void window::processEvents()
    {
        glfwPollEvents();
    }

bool window::isOpen() const
    {
        return !glfwWindowShouldClose(m_window);
    }

glm::vec2 window::getWindowSize() const
    {
        return glm::vec2(m_width, m_height);
    }

GLFWwindow *window::getUnderlyingWindow() const
    {
        return m_window;
    }
