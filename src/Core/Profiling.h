#pragma once
#define TRACY_ENABLE
#include "tracy/Tracy.hpp"

#define OMP_FRAME() FrameMark
#define OMP_STAT_SCOPE(Name) ZoneScopedN(Name)
#define OMP_STAT_THREAD_BEGIN(Name) FrameMarkStart(Name)
#define OMP_STAT_THREAD_END(Name) FrameMarkEnd(Name)
