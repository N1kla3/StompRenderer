#include "Core/Application.h"
#include <chrono>
#include <memory>

void omp::Application::start()
{
    using namespace std::chrono;

    preInit();
    init();

    float ms_limit = 1.f / m_FrameLimit;

    time_point previous = steady_clock::now();
    while (!m_RequestExit)
    {
        time_point current_time = steady_clock::now();
        float delta = duration_cast<milliseconds>(current_time - previous).count();
        current_time = previous;

        float delta_seconds = delta / 1000.f;
        
        tick(delta_seconds);

        if (m_FrameLimit > 0)
        {

        }
    }

    preDestroy();
}

void omp::Application::requestExit()
{
    m_RequestExit = true;
}

omp::Application::Application(const std::string& flags)
{
    parseFlags(flags);
}

void omp::Application::preInit()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_Window = glfwCreateWindow(m_Width, m_Height, "VulkanApplication", nullptr, nullptr);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, windowResizeCallback);


    if (m_ThreadCount > 0)
    {
        m_ThreadPool = std::make_unique<omp::ThreadPool>(static_cast<unsigned int>(m_ThreadCount));
    }
    else
    {
        m_ThreadPool = std::make_unique<omp::ThreadPool>();
    }
}

void omp::Application::init()
{
    m_AssetManager = std::make_unique<omp::AssetManager>(m_ThreadPool.get());
    m_Renderer = std::make_unique<omp::Renderer>();
}

void omp::Application::preDestroy()
{
    m_ThreadPool.reset();
}

void omp::Application::tick(float delta)
{
}

void omp::Application::parseFlags(const std::string& commands)
{
}

void omp::Application::windowResizeCallback(GLFWwindow* window, int width, int height)
{
    omp::Application* app = reinterpret_cast<omp::Application*>(glfwGetWindowUserPointer(window));
    // TODO: resize renderer
}

