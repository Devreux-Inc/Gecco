//
// Created by wylan on 12/19/24.
//

#ifndef common_h
#define common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NAN_BOXING
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC

#define UINT8_COUNT (UINT8_MAX + 1)

#ifdef __unix__
#elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define OS_Windows
#endif

bool isWindows();

#endif //common_h

#undef DEBUG_PRINT_CODE
#undef DEBUG_TRACE_EXECUTION
#undef DEBUG_STRESS_GC
#undef DEBUG_LOG_GC