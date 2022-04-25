#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace omp{

    void InitializeLogs();

}

#define INFO(Category, ...) spdlog::get(#Category)->info(__VA_ARGS__)
#define WARN(Category, ...) spdlog::get(#Category)->warn(__VA_ARGS__)
#define ERROR(Category, ...) spdlog::get(#Category)->error(__VA_ARGS__)
