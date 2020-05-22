#ifndef RUNEFLYFF_PLATFORM_THREADING_H
#define RUNEFLYFF_PLATFORM_THREADING_H
#pragma once

#ifdef __MINGW32__
#include "mingw/mingw.mutex.h"
#include "mingw/mingw.thread.h"
#else
#include <mutex>
#include <thread>
#endif

#endif //RUNEFLYFF_PLATFORM_THREADING_H
