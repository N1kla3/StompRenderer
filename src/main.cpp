
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "Logs.h"
#include "Renderer.h"

int main()
{
    omp::InitializeLogs();
    INFO(LogRendering, "=================Create Renderer=================");
    omp::Renderer application;

    try
    {
        application.run();
    }
    catch (const std::exception& e)
    {
        ERROR(LogRendering, e.what());
        return EXIT_FAILURE;
    }
    INFO(LogRendering, "================Destroy Renderer=================");
    return EXIT_SUCCESS;
}
