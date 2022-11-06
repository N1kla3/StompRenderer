
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "Logs.h"
#include "Renderer.h"

int main()
{
    omp::InitializeLogs();
    INFO(Rendering, "=================Create Renderer=================");
    Renderer application;

    try
    {
        application.run();
    }
    catch (const std::exception& e)
    {
        ERROR(Rendering, e.what());
        return EXIT_FAILURE;
    }
    INFO(Rendering, "================Destroy Renderer=================");
    return EXIT_SUCCESS;
}