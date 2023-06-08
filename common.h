// common.h

#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <unistd.h>

#include <time.h> // time
#include <sys/time.h> // gettimeofday
#include <string.h> // memset

#ifndef NO_STDLIB
  #include <stdlib.h>
#endif

#ifndef NO_STDIO
  #include <stdio.h>
#endif

#ifndef NO_SIMD
  #if __SSE__
    #define USE_SIMD
    #include <xmmintrin.h>
  #endif
#endif

typedef double f64;
typedef float f32;
typedef int64_t i64;
typedef uint64_t u64;
typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;

#define true 1
#define false 0

#define MAX_PATH_LENGTH 512
#define LENGTH(ARR) (sizeof(ARR) / sizeof(ARR[0]))

typedef enum { Ok = 0, Error } Result;

#if __APPLE__ || __MACH__
  #define PLATFORM_APPLE
#endif

#if __linux__
  #define PLATFORM_LINUX
#endif

#if _WIN32 || _WIN64
  #define PLATFORM_WINDOWS
  #include <windows.h>
#endif

#if _MSC_VER
  #define DEBUG_BREAK __debugbreak()
#elif defined(__has_builtin)
  #if __has_builtin(__builtin_debugtrap)
    #define DEBUG_BREAK __builtin_debugtrap()
  #elif __has_builtin(__builtin_trap)
    #define DEBUG_BREAK __builtin_trap()
  #endif
#endif

#ifndef DEBUG_BREAK
  #include <signal.h>
  #if defined(SIGTRAP)
    #define DEBUG_BREAK raise(SIGTRAP)
  #else
    #define DEBUG_BREAK raise(SIGABRT)
  #endif
#endif

#ifndef __FUNCTION_NAME__
  #ifdef PLATFORM_WINDOWS
    #define __FUNCTION_NAME__ __FUNCTION__
  #else
    #define __FUNCTION_NAME__ __func__
  #endif
#endif

#define ASSERT(...) \
  do { \
    if (!(__VA_ARGS__)) { \
      report_assert_failure(stderr, __FILE__, __LINE__, __FUNCTION_NAME__, #__VA_ARGS__); \
      DEBUG_BREAK; \
    } \
  } while (0)

#define TIMER_START(...) \
  struct timeval _end = {0}; \
  struct timeval _start = {0}; \
  gettimeofday(&_start, NULL); \
  __VA_ARGS__

#define TIMER_END(...) { \
  gettimeofday(&_end, NULL); \
  f32 dt = ((((_end.tv_sec - _start.tv_sec) * 1000000.0f) + _end.tv_usec) - (_start.tv_usec)) / 1000000.0f; \
  __VA_ARGS__ \
}

void report_assert_failure(FILE* fp, const char* filename, u32 line, const char* function_name, const char* message);

void report_assert_failure(FILE* fp, const char* filename, u32 line, const char* function_name, const char* message) {
  fprintf(fp, "[assert-fail]: %s:%u %s(): %s\n", filename, line, function_name, message);
}

#endif // _COMMON_H
