#include "Core/CoreLib.h"
#include <chrono>
#include <cstdint>

uint32_t omp::CoreLib::generateId32()
{
    using namespace std::chrono;

    static std::chrono::time_point<std::chrono::system_clock> diff_point = std::chrono::system_clock::now() - 10s;
    static uint8_t randomizer;
    while (randomizer > 0 && randomizer < 8)
    {
        randomizer++;
    }
    auto time = system_clock::now();
    time_t seconds = system_clock::to_time_t(time);
    uint32_t msec = static_cast<uint32_t>(duration_cast<microseconds>(time - diff_point).count());
    uint32_t thread_id = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    uint32_t result = 0;
    msec = msec << 20;
    uint32_t mask =  0x0FF000F0;
    uint32_t mask2 = 0x00FF0000;
    uint32_t mask3 = 0xFF000000;
    uint32_t mask4 = 0x0000FF00;
    uint32_t first = static_cast<uint32_t>(seconds) & mask;
    uint32_t second = msec & mask2;
    uint32_t third = 0x000000FF & thread_id;
    uint32_t four = (static_cast<uint32_t>(randomizer) << 28) & mask3;
    uint32_t five = (static_cast<uint32_t>(randomizer) << 8) & mask4;

    result = first ^ second ^ third ^ four ^ five;

    diff_point = system_clock::now();
    randomizer++;

    return result;
}

uint64_t omp::CoreLib::generateId64()
{
    using namespace std::chrono;

    static std::chrono::time_point<std::chrono::system_clock> diff_point = std::chrono::system_clock::now() - 10s;
    static uint16_t randomizer;
    while (randomizer > 0 && randomizer < 4)
    {
        randomizer++;
    }
    auto time = system_clock::now();
    time_t seconds = system_clock::to_time_t(time);
    uint64_t msec = static_cast<uint64_t>(duration_cast<microseconds>(time - diff_point).count());
    uint64_t thread_id = (std::hash<std::thread::id>{}(std::this_thread::get_id()));
    uint64_t result = 0;
    msec = msec << 48;
    uint64_t mask = 0x0000FF00FF00FFFF;
    uint64_t mask2 = 0xFFFF000000000000;
    uint64_t mask3 = 0x0000FFFF00000000;
    uint64_t mask4 = 0x00000000FFFF0000;
    uint64_t first = static_cast<uint64_t>(seconds) & mask;
    uint64_t second = msec & mask2;
    uint64_t third = 0x000000FF00FF0000 & thread_id;
    uint64_t four = (static_cast<uint64_t>(randomizer) << 32) & mask3;
    uint64_t five = (static_cast<uint64_t>(randomizer) << 16) & mask4;

    result = first ^ second ^ third ^ four ^ five;

    diff_point = system_clock::now();
    randomizer++;

    return result;
}
