#pragma once
#include "tracy/Tracy.hpp"

#ifdef TRACY_ENABLE
    #define OMP_FRAME() FrameMark
    #define OMP_STAT_SCOPE(Name) ZoneScopedN(Name)
    #define OMP_STAT_THREAD_BEGIN(Name) FrameMarkStart(Name)
    #define OMP_STAT_THREAD_END(Name) FrameMarkEnd(Name)
#else
    #define OMP_FRAME()
    #define OMP_STAT_SCOPE(Name)
    #define OMP_STAT_THREAD_BEGIN(Name)
    #define OMP_STAT_THREAD_END(Name)
#endif
