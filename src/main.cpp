
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "Logs.h"
#include "Core/Application.h"

int main()
{
    omp::InitializeLogs();
    INFO(LogRendering, "=================Create Application=================");
    omp::Application* application = new omp::Application{ "EMPTY FLAGS" };

    //try
    {
        application->start();
    }
    //catch (const std::exception& e)
    //{
    //    ERROR(LogRendering, e.what());
    //    return EXIT_FAILURE;
    //}
    delete application;
    INFO(LogRendering, "================Destroy Application=================");
    return EXIT_SUCCESS;
}
