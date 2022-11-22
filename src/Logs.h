#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace omp
{

    void InitializeLogs();
    void InitializeTestLogs();

}

// TODO: platform macros

#define INFO(Category, ...) spdlog::get(#Category)->info(__VA_ARGS__)
#define WARN(Category, ...) spdlog::get(#Category)->warn(__VA_ARGS__)
#define ERROR(Category, ...) spdlog::get(#Category)->error(__VA_ARGS__)

#define VINFO(Category, Message, ...) spdlog::get(#Category)->info(std::string("{} ") + std::string(Message), __FUNCTION__, __VA_ARGS__)
#define VWARN(Category, Message, ...) spdlog::get(#Category)->warn(std::string("{} ") + std::string(Message), __FUNCTION__, __VA_ARGS__)
#define VERROR(Category, Message, ...) spdlog::get(#Category)->error(std::string("{} ") + std::string(Message), __FUNCTION__, __VA_ARGS__)
