
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "Logs.h"
#include "Core/Application.h"

int main(int argc, char** argv)
{
    std::string project_absolute_path{};
    std::vector<std::string> flags;

    for (size_t index = 1; index < argc; index++)
    {
        if (index != 2)
        {
            flags.push_back(argv[index]);
        }
        else
        {
            project_absolute_path = argv[index];
        }
    }

    omp::InitializeLogs();
    INFO(LogRendering, "=================Create Application=================");
    omp::Application* application = new omp::Application{ project_absolute_path, flags };

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
