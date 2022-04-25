#include "Logs.h"

void omp::InitializeLogs()
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt"));
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    auto RenderingLog = std::make_shared<spdlog::logger>("Rendering", begin(sinks), end(sinks));
    auto UILog = std::make_shared<spdlog::logger>("UI", begin(sinks), end(sinks));
    spdlog::register_logger(RenderingLog);
    spdlog::register_logger(UILog);
    spdlog::set_pattern("[%D %H:%M:%S %z][thread %t]%^[%n][%l]%v%$");
}
