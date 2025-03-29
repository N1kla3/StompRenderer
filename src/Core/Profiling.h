// Copyright (C) 2021-2025 by Nikolay Vladimirskiy - kolya.vladimirsky@gmail.com
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

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
