#pragma once

#ifdef _MSC_VER
#include <Windows.h>
typedef unsigned __int64 TICKS;

#define	GetClockTicks	__rdtsc

#elif __GNUC__

typedef uint64_t TICKS;

#if defined(__aarch64__) || defined(__ARM_ARCH)
// ARM64 implementation using generic counter
#include <time.h>
__inline__ uint64_t GetClockTicks()
	{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
	}
#else
// x86/x86_64 implementation
__inline__ uint64_t GetClockTicks()
	{
	uint32_t lo, hi;
	/* We cannot use "=A", since this would use %rax on x86_64 */
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return (uint64_t)hi << 32 | lo;
	}
#endif

#else
#error	"getticks_h, unknown compiler"
#endif
