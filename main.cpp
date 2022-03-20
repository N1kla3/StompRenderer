
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Renderer.h"

int main()
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt"));
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    auto RenderingLog = std::make_shared<spdlog::logger>("Rendering", begin(sinks), end(sinks));
    auto UILog = std::make_shared<spdlog::logger>("UI", begin(sinks), end(sinks));
    spdlog::register_logger(RenderingLog);
    spdlog::register_logger(UILog);
    spdlog::set_pattern("[%D %H:%M:%S %z][thread %t]%^[%n][%l]%v%$");

    Renderer application;
    try{
        application.run();
    } catch (const std::exception& e){
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}